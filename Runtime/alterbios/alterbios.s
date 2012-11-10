	@ Generated code - see make_export.py
	@ This is the API symbols file for AlterBIOS.
	@ It is used from DSO Quad user applications to access AlterBIOS functions.

	.thumb
	.syntax unified

	.text

	.globl alterbios_version_tag
	. = 0x00000000
alterbios_version_tag:

	.globl patch_GetDev_SN
	.thumb_func
	. = 0x00000015
patch_GetDev_SN:

	.globl patch_OpenFileWr
	.thumb_func
	. = 0x00000019
patch_OpenFileWr:

	.globl patch_OpenFileRd
	.thumb_func
	. = 0x0000001d
patch_OpenFileRd:

	.globl patch_ReadFileSec
	.thumb_func
	. = 0x00000021
patch_ReadFileSec:

	.globl patch_ProgFileSec
	.thumb_func
	. = 0x00000025
patch_ProgFileSec:

	.globl patch_CloseFile
	.thumb_func
	. = 0x00000029
patch_CloseFile:

	.globl f_open
	.thumb_func
	. = 0x00000051
f_open:

	.globl f_read
	.thumb_func
	. = 0x00000055
f_read:

	.globl f_lseek
	.thumb_func
	. = 0x00000059
f_lseek:

	.globl f_close
	.thumb_func
	. = 0x0000005d
f_close:

	.globl f_opendir
	.thumb_func
	. = 0x00000061
f_opendir:

	.globl f_readdir
	.thumb_func
	. = 0x00000065
f_readdir:

	.globl f_stat
	.thumb_func
	. = 0x00000069
f_stat:

	.globl f_write
	.thumb_func
	. = 0x0000006d
f_write:

	.globl f_getfree
	.thumb_func
	. = 0x00000071
f_getfree:

	.globl f_truncate
	.thumb_func
	. = 0x00000075
f_truncate:

	.globl f_sync
	.thumb_func
	. = 0x00000079
f_sync:

	.globl f_unlink
	.thumb_func
	. = 0x0000007d
f_unlink:

	.globl f_mkdir
	.thumb_func
	. = 0x00000081
f_mkdir:

	.globl f_chmod
	.thumb_func
	. = 0x00000085
f_chmod:

	.globl f_utime
	.thumb_func
	. = 0x00000089
f_utime:

	.globl f_rename
	.thumb_func
	. = 0x0000008d
f_rename:

	.globl f_flush
	.thumb_func
	. = 0x00000091
f_flush:

