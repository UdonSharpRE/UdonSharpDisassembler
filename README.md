[中文](https://github.com/extremeblackliu/UdonSharpDisassembler/blob/main/README_cn.md)

# UdonSharpDisassembler
VRChat World Script UdonSharp(U#) Disassembler

# NOTE

the code was written with library [IL2CPP_Resolver](https://github.com/sneakyevil/IL2CPP_Resolver)

due to EAC(Easy Anti-Cheat) exists, so you need find the way use the code yourself. i will not give help

# Information

by the default, Disassembler will dump 3 files WITHOUT extension, they are ObjectName, bin_ObjectName, const_ObjectName

* ObjectName - contains Disassembled assembly code
* bin_ObjectName - contains origin Opcode of the UdonSharp Script
* const_ObjectName - contains every constants in the UdonSharp Script

this is examples output of those 3 files:

```
// ObjectName(example of Update function)
.func__update
0x00000000000003F0  PUSH 0x0000000000000013(__0_const_intnl_SystemUInt32[System.UInt32])
0x00000000000003F8  PUSH 0x0000000000000003(m_iCount[System.Int32])
0x0000000000000400  PUSH 0x000000000000000E(__1_const_intnl_SystemInt32[System.Int32])
0x0000000000000408  PUSH 0x0000000000000014(__3_intnl_SystemBoolean[System.Boolean])
0x0000000000000410  EXTERN "SystemInt32.__op_GreaterThan__SystemInt32_SystemInt32__SystemBoolean"
0x0000000000000418  PUSH 0x0000000000000014(__3_intnl_SystemBoolean[System.Boolean])
0x0000000000000420  JNE 0x000000000000043C
0x0000000000000428  PUSH 0x0000000000000009(__6_const_intnl_SystemInt32[System.Int32])
0x0000000000000430  PUSH 0x0000000000000003(m_iCount[System.Int32])
0x0000000000000438  COPY
0x000000000000043C  PUSH 0x0000000000000022(__0_intnl_returnTarget_UInt32[System.UInt32])
0x0000000000000444  COPY
0x0000000000000448  JMP __0_intnl_returnTarget_UInt32
.end
```

```
// bin_ObjectName(opened part of the binary with HxD Hex Editor)
00 00 00 01 00 00 00 13 00 00 00 01 00 00 00 07 00 00 00 01 00 00 00 02 00 00 00 09 00 00 00 01 00 00 00 0F 00 00 00 01 00 00 00 03 00 00 00 09 00 00 00 01 00 00 00 03 00 00 00 01 00 00 00 0E 00 00 00 01 00 00 00 17 00 00 00 06 00 00 00 23 00 00 00 01 00 00 00 17 00 00 00 04 00 00 01 40 00 00 00 01 00 00 00 03 00 00 00 01 00 00 00 0D 00 00 00 01 00 00 00 1C 00 00 00 06 00 00 00 24 00 00 00 01 00 00 00 1C 00 00 00 01 00 00 00 03 00 00 00 09 00 00 00 01 00 00 00 03 00 00 00 01 00 00 00 0C 00 00 00 01 00 00 00 16 00 00 00 06 00 00 00 25 00 00 00 01 00 00 00 16 00 00 00 04 00 00 01 38 00 00 00 01 00 00 00 03 00 00 00 01 00 00 00 0B 00 00 00 01 00 00 00 15 00 00 00 06 00 00 00 26 00 00 00 01 00 00 00 15 00 00 00 04 00 00 00 FC 00 00 00 05 00 00 01 38 00 00 00 01 00 00 00 03 00 00 00 01 00 00 00 1B 00 00 00 09 00 00 00 01 00 00 00 1B 00 00 00 01 00 00 00 0A 00 00 00 01 00 00 00 03 00 00 00 06 00 00 00 27 00 00 00 05 00 00 00 94 00 00 00 05 00 00 00 30 00 00 00 01 00 00 00 04 00 00 00 01 00 00 00 10 00 00 00 01 00 00 00 1F 00 00 00 06 00 00 00 28 00 00 00 01 00 00 00 1F 00 00 00 01 00 00 00 04 00 00 00 09 00 00 00 01 00 00 00 12 00 00 00 01
```

```
// const_ObjectName
0x0000000000000007 True
0x0000000000000008 20
0x0000000000000009 0
0x000000000000000A 1
0x000000000000000B 10
0x000000000000000C 23
0x000000000000000D 2
0x000000000000000E 100
0x000000000000000F 24
0x0000000000000010 2.000000
0x0000000000000011 "World"
0x0000000000000012 "Hello"
0x0000000000000013 4294967295
```

constants address are not disassembler address
