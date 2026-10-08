__int64 __fastcall render_create_default_textures(__int64 a1)
{
  __int64 v1; // r15
  unsigned int v3; // eax
  char v14; // bl
  unsigned int v29; // r13d
  void (__fastcall ***v36)(_QWORD); // rdi
  void (__fastcall ***v37)(_QWORD); // r12
  char v38; // r14
  void (__fastcall ***v39)(_QWORD); // rbx
  void (__fastcall ***v46)(_QWORD); // rdi
  void (__fastcall ***v47)(_QWORD); // r12
  void (__fastcall ***v48)(_QWORD); // rbx
  __int64 v55; // rbx
  __int64 v56; // rbx
  __int64 v57; // rbx
  __int64 v58; // rbx
  __int64 v59; // rbx
  __int64 v60; // rbx
  void (__fastcall ***v64)(_QWORD); // rbx
  void (__fastcall ***v65)(_QWORD); // r14
  void (__fastcall ***v66)(_QWORD); // rbx
  void (__fastcall ***v67)(_QWORD); // r14
  __int64 v69; // [rsp+18h] [rbp-7F8h]
  char v70; // [rsp+24h] [rbp-7ECh]
  unsigned int v72; // [rsp+30h] [rbp-7E0h]
  int v73; // [rsp+34h] [rbp-7DCh]
  _BYTE v74[4]; // [rsp+38h] [rbp-7D8h] BYREF
  int v75; // [rsp+3Ch] [rbp-7D4h]
  char v76; // [rsp+40h] [rbp-7D0h] BYREF
  __int64 v77; // [rsp+5Ch] [rbp-7B4h]
  int v78; // [rsp+64h] [rbp-7ACh]
  __int64 v79; // [rsp+C0h] [rbp-750h]
  _BYTE v80[80]; // [rsp+C8h] [rbp-748h] BYREF
  _BYTE v81[4]; // [rsp+118h] [rbp-6F8h] BYREF
  int v82; // [rsp+11Ch] [rbp-6F4h]
  char v83; // [rsp+120h] [rbp-6F0h] BYREF
  __int64 v84; // [rsp+13Ch] [rbp-6D4h]
  int v85; // [rsp+144h] [rbp-6CCh]
  __int64 v86; // [rsp+1A0h] [rbp-670h]
  _BYTE v87[80]; // [rsp+1A8h] [rbp-668h] BYREF
  _BYTE v88[4]; // [rsp+1F8h] [rbp-618h] BYREF
  int v89; // [rsp+1FCh] [rbp-614h]
  char v90; // [rsp+200h] [rbp-610h] BYREF
  __int64 v91; // [rsp+21Ch] [rbp-5F4h]
  int v92; // [rsp+224h] [rbp-5ECh]
  __int64 v93; // [rsp+280h] [rbp-590h]
  _BYTE v94[80]; // [rsp+288h] [rbp-588h] BYREF
  _BYTE v95[40]; // [rsp+2D8h] [rbp-538h] BYREF
  __int128 v96; // [rsp+300h] [rbp-510h] BYREF
  __int128 v97; // [rsp+310h] [rbp-500h] BYREF
  __int128 v99; // [rsp+340h] [rbp-4D0h] BYREF
  int v100; // [rsp+350h] [rbp-4C0h]
  _BYTE v101[4]; // [rsp+360h] [rbp-4B0h] BYREF
  int v102; // [rsp+364h] [rbp-4ACh]
  char v103; // [rsp+368h] [rbp-4A8h] BYREF
  __int64 v104; // [rsp+384h] [rbp-48Ch]
  int v105; // [rsp+38Ch] [rbp-484h]
  __int64 v106; // [rsp+3E8h] [rbp-428h]
  _QWORD v107[4]; // [rsp+3F0h] [rbp-420h] BYREF
  _BYTE v108[4]; // [rsp+410h] [rbp-400h] BYREF
  int v109; // [rsp+414h] [rbp-3FCh]
  char v110; // [rsp+418h] [rbp-3F8h] BYREF
  __int64 v111; // [rsp+434h] [rbp-3DCh]
  int v112; // [rsp+43Ch] [rbp-3D4h]
  __int64 v113; // [rsp+498h] [rbp-378h]
  void (__fastcall ***v114)(_QWORD); // [rsp+4A0h] [rbp-370h] BYREF
  void (__fastcall ***v115)(_QWORD); // [rsp+4A8h] [rbp-368h]
  __int64 v116; // [rsp+4B0h] [rbp-360h]
  char v117[8]; // [rsp+4B8h] [rbp-358h] BYREF
  _BYTE v118[4]; // [rsp+4C0h] [rbp-350h] BYREF
  int v119; // [rsp+4C4h] [rbp-34Ch]
  char v120; // [rsp+4C8h] [rbp-348h] BYREF
  __int64 v121; // [rsp+4E4h] [rbp-32Ch]
  int v122; // [rsp+4ECh] [rbp-324h]
  __int64 v123; // [rsp+548h] [rbp-2C8h]
  void (__fastcall ***v124)(_QWORD); // [rsp+550h] [rbp-2C0h] BYREF
  void (__fastcall ***v125)(_QWORD); // [rsp+558h] [rbp-2B8h]
  __int64 v126; // [rsp+560h] [rbp-2B0h]
  char v127[8]; // [rsp+568h] [rbp-2A8h] BYREF
  _BYTE v128[4]; // [rsp+570h] [rbp-2A0h] BYREF
  int v129; // [rsp+574h] [rbp-29Ch]
  char v130; // [rsp+578h] [rbp-298h] BYREF
  __int64 v131; // [rsp+594h] [rbp-27Ch]
  int v132; // [rsp+59Ch] [rbp-274h]
  __int64 v133; // [rsp+5F8h] [rbp-218h]
  _BYTE v134[80]; // [rsp+600h] [rbp-210h] BYREF
  _BYTE v135[80]; // [rsp+650h] [rbp-1C0h] BYREF
  _BYTE v136[80]; // [rsp+6A0h] [rbp-170h] BYREF
  _BYTE v137[80]; // [rsp+6F0h] [rbp-120h] BYREF
  _BYTE v138[80]; // [rsp+740h] [rbp-D0h] BYREF
  _BYTE v139[80]; // [rsp+790h] [rbp-80h] BYREF
  __int64 v140; // [rsp+7E0h] [rbp-30h]

  v1 = 0;
  v140 = 0x6365786562696C2FLL;
  do
  {
    __asm { vmovups xmm0, cs:xmmword_12B6B40 }
    v100 = -1;
    __asm { vmovups [rbp+var_4D0], xmm0 }
    v3 = sub_68E4D0((unsigned int *)&v99, 7u);
    __asm
    {
      vmovups xmm1, cs:xmmword_12B6B50
      vxorps  xmm0, xmm0, xmm0
    }
    v72 = v3;
    __asm
    {
      vmovups [rbp+var_4E4], xmm0
      vmovups xmmword ptr [rbp-4F0h], xmm0
      vmovups [rbp+var_500], xmm1
      vmovups [rbp+var_510], xmm1
    }
    switch ( (int)v1 )
    {
      case 0:
        if ( byte_19E3F50[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19E3F50) != 0 )
        {
          __asm { vmovups xmm0, cs:xmmword_12B6BD0 }
          _RAX = &unk_19E3F40;
          __asm { vmovups xmmword ptr [rax], xmm0 }
          _cxa_guard_release(byte_19E3F50);
        }
        _RAX = &unk_19E3F40;
        goto LABEL_23;
      case 1:
        if ( byte_19E4A18[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19E4A18) != 0 )
        {
          __asm { vmovups xmm0, cs:xmmword_12B6B50 }
          _RAX = &unk_19E4A08;
          __asm { vmovups xmmword ptr [rax], xmm0 }
          _cxa_guard_release(byte_19E4A18);
        }
        _RAX = &unk_19E4A08;
        goto LABEL_23;
      case 2:
        if ( byte_1A712C8[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_1A712C8) != 0 )
        {
          _RAX = &unk_1A712B8;
          __asm { vxorps  xmm0, xmm0, xmm0 }
          __asm { vmovups xmmword ptr [rax], xmm0 }
          _cxa_guard_release(byte_1A712C8);
        }
        _RAX = &unk_1A712B8;
LABEL_23:
        __asm
        {
          vmovups xmm0, xmmword ptr [rax]
          vmovups [rbp+var_500], xmm0
        }
        goto LABEL_24;
      case 3:
        __asm { vmovups xmm0, cs:xmmword_12B6BC0; jumptable 00000000006BDF18 case 3 }
        v73 = 3;
        v14 = 0;
        __asm { vmovups [rbp+var_500], xmm0 }
        break;
      case 4:
        if ( byte_1A76580[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_1A76580) != 0 )
        {
          __asm { vmovups xmm0, cs:xmmword_12B6BA0 }
          _RAX = &unk_1A76570;
          __asm { vmovups xmmword ptr [rax], xmm0 }
          _cxa_guard_release(byte_1A76580);
        }
        _RAX = &unk_1A76570;
        __asm
        {
          vmovups xmm0, xmmword ptr [rax]
          vmovups [rbp+var_500], xmm0
        }
        if ( byte_1A6EF50[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_1A6EF50) != 0 )
        {
          __asm { vmovups xmm0, cs:xmmword_12B6BB0 }
          _RAX = &unk_1A6EF40;
          __asm { vmovups xmmword ptr [rax], xmm0 }
          _cxa_guard_release(byte_1A6EF50);
        }
        _RAX = &unk_1A6EF40;
        __asm { vmovups xmm0, xmmword ptr [rax] }
        goto LABEL_26;
      case 5:
        __asm
        {
          vmovups xmm0, cs:xmmword_12B6B80; jumptable 00000000006BDF18 case 5
          vmovups [rbp+var_500], xmm0
          vmovups xmm0, cs:xmmword_12B6B90
        }
LABEL_26:
        __asm { vmovups [rbp+var_510], xmm0 }
        v14 = 1;
        v73 = 0;
        break;
      case 6:
        __asm
        {
          vmovups xmm0, cs:xmmword_12B6B60; jumptable 00000000006BDF18 case 6
          vmovups xmm1, cs:xmmword_12B6B70
        }
        v14 = 1;
        v73 = 3;
        __asm
        {
          vmovups [rbp+var_500], xmm0
          vmovups [rbp+var_510], xmm1
        }
        break;
      default:
LABEL_24:
        v73 = 0;
        v14 = 0;
        break;
    }
    v29 = 8;
    if ( v14 != 0 )
      v29 = 64;
    v69 = sub_6AC700(v1);
    sub_681630(v95);
    sub_681C00((__int64)v95, v29, 1, 1, (__int64)&v97);
    v70 = v14;
    if ( v14 != 0 )
      sub_6AED00(v95, &v97, &v96, 8);
    sub_6F5A90(v88);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RDI = &v90;
    v93 = v69;
    v89 = v73;
    __asm
    {
      vmovups xmmword ptr [rdi+0Ch], xmm1
      vmovups xmmword ptr [rdi], xmm0
    }
    v91 = 0x100000002LL;
    v92 = 0;
    sub_6830D0(v94, v29, 1, 1, v72);
    sub_684960(v94, v95);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 8) = sub_6F5810((__int64)v88, 0);
    sub_6972D0(v118);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RCX = &v120;
    v123 = v69;
    v119 = v73;
    __asm
    {
      vmovups xmmword ptr [rcx+0Ch], xmm1
      vmovups xmmword ptr [rcx], xmm0
    }
    v121 = 0x100000002LL;
    v122 = 0;
    v36 = v124;
    v37 = v125;
    if ( v125 == v124 )
    {
      sub_48E910(&v124, 1);
      v36 = v124;
      v38 = v14;
    }
    else
    {
      v38 = v14;
      v39 = v124 + 10;
      if ( v124 + 10 != v125 )
      {
        do
        {
          (**v39)(v39);
          v39 += 10;
        }
        while ( v37 != v39 );
        v36 = v124;
      }
      v125 = v36 + 10;
    }
    sub_683000(v36, v29, 1, 1, v72, 0);
    sub_6836B0(v124, v94);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 232) = sub_696BA0((__int64)v118, 0);
    sub_681C00((__int64)v95, v29, v29, 1, (__int64)&v97);
    if ( v38 != 0 )
      sub_6AED00(v95, &v97, &v96, 8);
    sub_690330(v81);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RAX = &v83;
    v86 = v69;
    v82 = v73;
    __asm
    {
      vmovups xmmword ptr [rax+0Ch], xmm1
      vmovups xmmword ptr [rax], xmm0
    }
    v84 = 0x100000002LL;
    v85 = 0;
    sub_6830D0(v87, v29, v29, 1, v72);
    sub_684960(v87, v95);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 64) = sub_68FE80((__int64)v81, 0);
    sub_698770(v108);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RAX = &v110;
    v113 = v69;
    v109 = v73;
    __asm
    {
      vmovups xmmword ptr [rax+0Ch], xmm1
      vmovups xmmword ptr [rax], xmm0
    }
    v111 = 0x100000002LL;
    v112 = 0;
    v46 = v114;
    v47 = v115;
    if ( v115 == v114 )
    {
      sub_48E910(&v114, 1);
      v46 = v114;
    }
    else
    {
      v48 = v114 + 10;
      if ( v114 + 10 != v115 )
      {
        do
        {
          (**v48)(v48);
          v48 += 10;
        }
        while ( v47 != v48 );
        v46 = v114;
      }
      v115 = v46 + 10;
    }
    sub_683000(v46, v29, v29, 1, v72, 0);
    sub_6836B0(v114, v87);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 288) = sub_698100((__int64)v108, 0);
    sub_6A1030(v128);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RDI = &v130;
    v133 = v69;
    v129 = v73;
    __asm
    {
      vmovups xmmword ptr [rdi+0Ch], xmm1
      vmovups xmmword ptr [rdi], xmm0
    }
    v131 = 0x100000002LL;
    v132 = 0;
    sub_683000(v134, v29, v29, 1, v72, 0);
    sub_6836B0(v134, v87);
    sub_683000(v135, v29, v29, 1, v72, 0);
    sub_6836B0(v135, v87);
    sub_683000(v136, v29, v29, 1, v72, 0);
    sub_6836B0(v136, v87);
    sub_683000(v137, v29, v29, 1, v72, 0);
    sub_6836B0(v137, v87);
    sub_683000(v138, v29, v29, 1, v72, 0);
    sub_6836B0(v138, v87);
    sub_683000(v139, v29, v29, 1, v72, 0);
    sub_6836B0(v139, v87);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 176) = sub_6A0D10((__int64)v128, 0);
    sub_69AF00((__int64)v101);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _RDX = &v103;
    v106 = v69;
    v102 = v73;
    __asm
    {
      vmovups xmmword ptr [rdx+0Ch], xmm1
      vmovups xmmword ptr [rdx], xmm0
    }
    v104 = 0x100000002LL;
    v105 = 0;
    sub_49AA20(v107, 1);
    v55 = v107[0];
    sub_683000(v107[0], v29, v29, 1, v72, 0);
    sub_6836B0(v55, v87);
    v56 = v107[0] + 80LL;
    sub_683000(v107[0] + 80LL, v29, v29, 1, v72, 0);
    sub_6836B0(v56, v87);
    v57 = v107[0] + 160LL;
    sub_683000(v107[0] + 160LL, v29, v29, 1, v72, 0);
    sub_6836B0(v57, v87);
    v58 = v107[0] + 240LL;
    sub_683000(v107[0] + 240LL, v29, v29, 1, v72, 0);
    sub_6836B0(v58, v87);
    v59 = v107[0] + 320LL;
    sub_683000(v107[0] + 320LL, v29, v29, 1, v72, 0);
    sub_6836B0(v59, v87);
    v60 = v107[0] + 400LL;
    sub_683000(v107[0] + 400LL, v29, v29, 1, v72, 0);
    sub_6836B0(v60, v87);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 344) = sub_69AA60((__int64)v101, 0);
    sub_681C00((__int64)v95, v29, v29, v29, (__int64)&v97);
    if ( v70 != 0 )
      sub_6AED00(v95, &v97, &v96, 8);
    sub_6F5ED0(v74);
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-4F0h]
      vmovups xmm1, [rbp+var_4E4]
    }
    _R9 = &v76;
    v79 = v69;
    v75 = v73;
    __asm
    {
      vmovups xmmword ptr [r9+0Ch], xmm1
      vmovups xmmword ptr [r9], xmm0
    }
    v77 = 0x100000002LL;
    v78 = 0;
    sub_6830D0(v80, v29, v29, v29, v72);
    sub_684960(v80, v95);
    *(_QWORD *)(a1 + 8LL * (int)v1 + 120) = sub_6F5C50((__int64)v74, 0);
    sub_682BC0(v80);
    sub_48D3D0(v107);
    sub_682BC0(v139);
    sub_682BC0(v138);
    sub_682BC0(v137);
    sub_682BC0(v136);
    sub_682BC0(v135);
    sub_682BC0(v134);
    v64 = v114;
    v65 = v115;
    if ( v114 == v115 )
    {
      if ( v114 == nullptr )
        goto LABEL_54;
LABEL_53:
      sub_252D30(v117, v64, v116 - (_QWORD)v64);
      goto LABEL_54;
    }
    do
    {
      (**v64)(v64);
      v64 += 10;
    }
    while ( v65 != v64 );
    v64 = v114;
    if ( v114 != nullptr )
      goto LABEL_53;
LABEL_54:
    sub_682BC0(v87);
    v66 = v124;
    v67 = v125;
    if ( v124 != v125 )
    {
      do
      {
        (**v66)(v66);
        v66 += 10;
      }
      while ( v67 != v66 );
      v66 = v124;
    }
    if ( v66 != nullptr )
      sub_252D30(v127, v66, v126 - (_QWORD)v66);
    sub_682BC0(v94);
    sub_681800(v95);
    ++v1;
  }
  while ( v1 != 7 );
  return 0x6365786562696C2FLL;
}
