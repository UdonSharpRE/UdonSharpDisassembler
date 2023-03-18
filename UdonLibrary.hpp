namespace Disassembler {

		enum OPCODE {
			NOP,
			PUSH,
			POP,
			JNE = 4, //JUMP_IF_FALSE
			JMP,
			EXTERN,
			ANNOTATION,
			JUMP_INDIRECT,
			COPY,
		};

		std::string SFormat(const char* m_pBuffer, ...)
		{
			ImGuiTextBuffer m_Buffer;
			va_list args;
			va_start(args, m_pBuffer);

			m_Buffer.appendfv(m_pBuffer, args);
			m_Buffer.append("\n");

			va_end(args);

			std::string m_sRet(m_Buffer.c_str(), m_Buffer.size());

			m_Buffer.clear();

			return m_sRet;
		}
		std::string SFormatNoNewLine(const char* m_pBuffer, ...)
		{
			ImGuiTextBuffer m_Buffer;
			va_list args;
			va_start(args, m_pBuffer);

			m_Buffer.appendfv(m_pBuffer, args);

			va_end(args);

			std::string m_sRet(m_Buffer.c_str(), m_Buffer.size());

			m_Buffer.clear();

			return m_sRet;
		}
		void Save(std::string sFileName, std::string sData)
		{
			std::ofstream os("D:\\" + sFileName);

			os.write(sData.c_str(), sData.size());

			os.close();	
		}
		void SaveBinary(std::string sFileName, unsigned char* pData, size_t iLength)
		{
			std::ofstream os("D:\\bin_" + sFileName, std::ios::binary);

			os.write((const char*)pData, iLength);

			os.close();
		}

		bool IsInASCII(std::string sString)
		{
			for (auto& i : sString)
			{
				if ((unsigned char)i > 127)
				{
					return false;
				}
			}
			return true;
		}

		void Disassembly(std::string sObjectPath)
		{
			Unity::CGameObject* UdonObject = Unity::GameObject::Find(sObjectPath.c_str());
			if (!UdonObject)
				return;

			Unity::CComponent* UdonBehaviour = UdonObject->GetComponent("UdonBehaviour");
			if (!UdonBehaviour)
				return;

			Unity::CComponent* UdonProgram = UdonBehaviour->GetMemberValue<Unity::CComponent*>("_program");
			if (!UdonProgram)
				return;

			Unity::CComponent* UdonSymbolTable = UdonProgram->GetMemberValue<Unity::CComponent*>("SymbolTable");
			if (!UdonSymbolTable)
				return;

			Unity::CComponent* UdonHeap = UdonProgram->GetMemberValue<Unity::CComponent*>("Heap");
			if (!UdonHeap)
				return;

			Unity::il2cppArray<unsigned char>* ByteCode = UdonProgram->GetMemberValue<Unity::il2cppArray<unsigned char>*>("ByteCode");
			if (!ByteCode)
				return;

			auto _eventTable = UdonBehaviour->GetMemberValue<Unity::il2cppDictionary<Unity::System_String*, Unity::il2cppList<unsigned int>*>*>("_eventTable");

			std::string m_sDisassembled;

			Logger::AddLog("Disassembling OpCode...");

			Logger::AddLog("Setting up direct OpCode");
			// Setup Code in Memory so we can directly dereference
			unsigned char* CodeBase = (unsigned char*)VirtualAlloc(0x0, ByteCode->m_uMaxLength, MEM_COMMIT, PAGE_READWRITE);
			if (!CodeBase)
			{
				Logger::AddLog("Setting up OpCode Failed!");
				return;
			}

			for (int i = 0; i < ByteCode->m_uMaxLength; i++)
			{
				unsigned char code = ByteCode->operator[](i);
				CodeBase[i] = code;
			}

			SaveBinary(UdonObject->GetName()->ToString(), CodeBase, ByteCode->m_uMaxLength);
			Logger::AddLog("Saved Binary!");

			std::map<int, std::string> Constants;

			// Resolve Opcode
			unsigned int* Code = (unsigned int*)CodeBase;
			
			bool m_bSkipNext = false;

			int m_iAddress = 0;
			for (int i = 0; i < ByteCode->m_uMaxLength / 4; i++)
			{
				if (m_iAddress == ByteCode->m_uMaxLength)
				{
					m_sDisassembled += SFormat(".end");
				}
				if (m_bSkipNext)
				{
					m_bSkipNext = false;
					continue;
				}
				
				// look up eventTables if match any address
				if (_eventTable)
				{
					bool m_bIsFunc = false;
					std::string m_sFuncSymbol;

					for (int i = 0; i < _eventTable->m_iCount; i++)
					{
						auto key = _eventTable->GetKeyByIndex(i);
						auto values = _eventTable->GetValueByKey(key);
						if (!key || !values)
							continue;

						Unity::il2cppArray<unsigned int>* m_pAddress = values->ToArray();
						if (m_pAddress->operator[](0) == m_iAddress)
						{
							m_sFuncSymbol = key->ToString();
							m_bIsFunc = true;
							break;
						}
					}
					if (m_bIsFunc)
					{
						if(m_iAddress > 0) m_sDisassembled += SFormat(".end\n");
						if (IsInASCII(m_sFuncSymbol))
						{
							m_sDisassembled += SFormat(".func_%s", m_sFuncSymbol);
						}
						else {
							m_sDisassembled += SFormat(".func_%d", m_iAddress);
						}
					}
				}

				unsigned int opcode = _byteswap_ulong(Code[i]);
				switch (opcode)
				{
				case OPCODE::NOP:
				{
					Logger::AddLog("Resolving NOP");
					// maybe ignore if it doesnt do shit
					m_sDisassembled += SFormat("0x%p  NOP", m_iAddress);

					m_iAddress += 4;
				}
				break;
				case OPCODE::PUSH:
				{
					/*
						in UdonSharp, first pushed value is first argument value, its inverted with native assembly
					*/

					Logger::AddLog("Resolving PUSH");
					// if there is push instruction it should be have more data, or the opcode is invalid
					unsigned int PushOffset = _byteswap_ulong(Code[i + 1]);

					std::string m_sSymbol = UdonSymbolTable->CallMethod<Unity::System_String*>("GetSymbolFromAddress", PushOffset)->ToString();
					
					std::string m_sType = "Missing";

					auto RuntimeType = UdonHeap->CallMethod<Unity::CComponent*>("GetHeapVariableType", PushOffset);
					if (RuntimeType)
					{
						auto m_sTypeFullName = RuntimeType->GetMemberValue<Unity::System_String*>("FullName");
						if (m_sTypeFullName)
						{
							m_sType = m_sTypeFullName->ToString();
							if (m_sSymbol.find("_const_") != std::string::npos)
							{
								if (m_sType.find("Boolean") != std::string::npos)
								{
									Constants[PushOffset] = UdonHeap->CallMethod<Unity::CComponent*>("GetHeapVariable", 1, PushOffset)->CallMethod<Unity::System_String*>("ToString")->ToString();
								}
								if (m_sType.find("Int32") != std::string::npos)
								{
									Constants[PushOffset] = UdonHeap->CallMethod<Unity::CComponent*>("GetHeapVariable", 1, PushOffset)->CallMethod<Unity::System_String*>("ToString")->ToString();
								}
								if (m_sType.find("Single") != std::string::npos)
								{
									Constants[PushOffset] = std::to_string(UdonHeap->CallMethod<float>("GetHeapVariable", 1, PushOffset));
								}
								if (m_sType.find("Double") != std::string::npos)
								{
									Constants[PushOffset] = std::to_string(UdonHeap->CallMethod<double>("GetHeapVariable", 1, PushOffset));
								}
								if (m_sType.find("String") != std::string::npos)
								{
									Constants[PushOffset] = SFormatNoNewLine("\"%s\"", UdonHeap->CallMethod<Unity::System_String*>("GetHeapVariable", PushOffset)->ToString().c_str());
								}
							}
						}
					}

					m_sDisassembled += SFormat("0x%p  PUSH 0x%p(%s[%s])", m_iAddress, PushOffset, m_sSymbol.c_str(), m_sType.c_str());
					
					// instruction cost 2x sizeof(int)
					m_iAddress += 8;

					m_bSkipNext = true;
				}
				break;
				case OPCODE::POP:
				{
					Logger::AddLog("Resolving POP");
					m_sDisassembled += SFormat("0x%p  POP", m_iAddress);

					m_iAddress += 4;
				}
				break;
				case OPCODE::JNE:
				{
					Logger::AddLog("Resolving JNE");
					unsigned int JumpOffset = _byteswap_ulong(Code[i + 1]);

					m_sDisassembled += SFormat("0x%p  JNE 0x%p", m_iAddress, JumpOffset);

					m_iAddress += 8;

					m_bSkipNext = true;
				}
				break;
				case OPCODE::JMP:
				{
					Logger::AddLog("Resolving JMP");
					unsigned int JumpOffset = _byteswap_ulong(Code[i + 1]);

					m_sDisassembled += SFormat("0x%p  JMP 0x%p", m_iAddress, JumpOffset);

					m_iAddress += 8;

					m_bSkipNext = true;
					/* UdonGraph only and incorrect way
					// detect function ending jump
					if (JumpOffset == 0xFFFFFFFC)
					{
						m_sDisassembled += SFormat(".end\n");
					}
					*/
				}
				break;
				case OPCODE::EXTERN:
				{
					Logger::AddLog("Resolving EXTERN");
					unsigned int ExternAddress = _byteswap_ulong(Code[i + 1]);
	
					std::string m_sExternName = "!Resolve Extern Failed!";
					
					Unity::CObject* m_pComponent = UdonHeap->CallMethod<Unity::CObject*>("GetHeapVariable", ExternAddress);
					if (m_pComponent)
					{
						auto RuntimeType = m_pComponent->CallMethod<Unity::CComponent*>("GetType");
						if (RuntimeType)
						{
							auto m_sTypeFullName = RuntimeType->GetMemberValue<Unity::System_String*>("FullName");
							if (m_sTypeFullName)
							{
								if (strstr(m_sTypeFullName->ToString().c_str(), "UdonExternDelegate"))
								{
									m_sExternName = ((Unity::CComponent*)m_pComponent)->GetMemberValue<Unity::System_String*>("externSignature")->ToString();
								}
								else
								{
									m_sExternName = ((Unity::System_String*)m_pComponent)->ToString();
								}
							}
						}
					}

					m_sDisassembled += SFormat("0x%p  EXTERN \"%s\"", m_iAddress, m_sExternName.c_str());

					m_iAddress += 8;

					m_bSkipNext = true;
				}
				break;
				case OPCODE::ANNOTATION:
				{
					Logger::AddLog("Resolving ANNOTATION");
					unsigned int ExternAddress = _byteswap_ulong(Code[i + 1]);

					std::string m_sExternName;

					Unity::System_String* m_pComponent = UdonHeap->CallMethod<Unity::System_String*>("GetHeapVariable", ExternAddress);
					if (!m_pComponent)
					{
						m_sExternName = "!Resolve Extern Failed!";
					}
					else
					{
						m_sExternName = m_pComponent->ToString();
					}

					m_sDisassembled += SFormat("0x%p  ANNOTATION \"%s\"", m_iAddress, m_sExternName.c_str());

					m_iAddress += 8;

					m_bSkipNext = true;
				}
				break;
				case OPCODE::JUMP_INDIRECT:
				{
					Logger::AddLog("Resolving JMP[]");
					unsigned int JumpInDirect = _byteswap_ulong(Code[i + 1]);

					std::string sSymbol = "N/A";

					if (UdonSymbolTable->CallMethod<bool>("HasSymbolForAddress"))
					{
						sSymbol = UdonSymbolTable->CallMethod<Unity::System_String*>("GetSymbolFromAddress", JumpInDirect)->ToString();
					}
					else {
						sSymbol = SFormat("0x%p", JumpInDirect);
					}

					m_sDisassembled += SFormat("0x%p  JMP %s", m_iAddress, sSymbol.c_str());

					m_iAddress += 8;

					m_bSkipNext = true;
				}
				break;
				case OPCODE::COPY:
				{
					Logger::AddLog("Resolving COPY");
					m_sDisassembled += SFormat("0x%p  COPY", m_iAddress);

					m_iAddress += 4;
				}
				break;
				}
			}

			VirtualFree(CodeBase, 0x0, MEM_DECOMMIT);
			Logger::AddLog("Code has been Free...");

			Logger::AddLog("Disassembly finished, saving to File...");

			Save(UdonObject->GetName()->ToString(), m_sDisassembled);
			
			Logger::AddLog("File Saved.");
		

			Logger::AddLog("Dumping Constants...");

			std::string m_sConstants;
			for (const auto& [key, value] : Constants) {
				m_sConstants += SFormat("0x%p %s", key, value.c_str());
			}

			Save(std::string("const_") + UdonObject->GetName()->ToString(), m_sConstants);
		}
	}
