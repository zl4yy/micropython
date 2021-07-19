/*

	Basic SD Card support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	26 June 2021
	
	Software provided under MIT License

*/
#include <stdbool.h>
#include "modules/sdcard.h"

#include "py/compile.h"
#include "py/objstr.h"
//#include "py/persistentcode.h"
//#include "py/runtime.h"
//#include "py/repl.h"
//#include "py/mperrno.h"
//#include "lib/utils/pyexec.h"

bool SDCard_InitDone = false;

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t sdcard_info(void) {
    mp_printf(&mp_plat_print, "Start with sdcard.init(<spiport>) and sdcard.listdir()\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(sdcard_info_obj, sdcard_info);

// Initialise access to SD Card
STATIC mp_obj_t sdcard_init(mp_obj_t ssinum_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);

    Do_SysTick_Init();
    Do_SD_SetPins(ssinum);

    // Using SSI 3 Master, SPI frame format, 250 Kbps, 8 data bits
    // LM4F's SSI support 32 bits frames but the code is currently writen to shift 8 bits at a time
    Do_SSI_Init(ssinum, 10001, true);

	if (Do_SD_initialise()) {
        Do_SD_cs_high();
        // Using SSI 3 Master, SPI frame format, 8 Mbps, 8 data bits.
        // 8 Mbps is conservative but should work with all SD cards and is plenty enough for most applications
        // LM4F's SSI support 32 bits frames but the code is currently writen to shift 8 bits at a time
        // NOTE: Settings must be consistent with Do_SD_initialise in sdcard.c used for high speed reset
        Do_SSI_Init(ssinum, 20051, true);
        Do_SD_tx_SSI();

        Do_SD_cs_low();
        Do_SD_read_first_sector();
        Do_SD_read_disk_data();

        mp_printf(&mp_plat_print, "SD Card initialised.\n");

    } else {
        mp_printf(&mp_plat_print, "SD Card init error.\n");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_init_obj, sdcard_init);

// Read directory from SD Card
STATIC mp_obj_t sdcard_listdir(void) {
    long next_cluster=Do_SD_get_root_dir_first_cluster();

    do {
            next_cluster=Do_SD_get_files_and_dirs(next_cluster, LONG_NAME, GET_SUBDIRS, true);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(sdcard_listdir_obj, sdcard_listdir);

// Print file from SD Card
STATIC mp_obj_t sdcard_printfile(mp_obj_t filenum_obj) {

    uint8_t filenum = mp_obj_get_int(filenum_obj);

    long next_cluster=Do_SD_get_first_cluster(filenum);
    
    do {
        next_cluster=Do_SD_print_file(next_cluster);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_printfile_obj, sdcard_printfile);

// Print binary file from SD Card
STATIC mp_obj_t sdcard_printfilebin(mp_obj_t filenum_obj) {

    uint8_t filenum = mp_obj_get_int(filenum_obj);

    long next_cluster=Do_SD_get_first_cluster(filenum);
    
    do {
        next_cluster=Do_SD_print_filebin(next_cluster);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_printfilebin_obj, sdcard_printfilebin);

// Print file from SD Card searching by name
STATIC mp_obj_t sdcard_printfilebyname(const mp_obj_t filename_obj) {
    mp_check_self(mp_obj_is_str_or_bytes(filename_obj));

    GET_STR_DATA_LEN(filename_obj, str, str_len);
    char filename_str[str_len];
    strcpy(filename_str, (char *)str);

    uint8_t filenum = Do_SD_find_file_by_name(filename_str);

    if (filenum<40) {
        long next_cluster=Do_SD_get_first_cluster(filenum);
        do {
            next_cluster=Do_SD_print_file(next_cluster);
        } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);
    } else {
        mp_printf(&mp_plat_print, "File not found.\n");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_printfilebyname_obj, sdcard_printfilebyname);

// Read file from SD Card (return text as string)
STATIC mp_obj_t sdcard_readfile(mp_obj_t filenum_obj) {

    uint8_t filenum = mp_obj_get_int(filenum_obj);

    uint16_t offset=0;
	uint8_t source[BUFFERSIZE];

    long next_cluster=Do_SD_get_first_cluster(filenum);

    do {
        next_cluster=Do_SD_read_file(next_cluster, &source[offset], &offset);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF && offset < BUFFERSIZE);

    if (offset>=BUFFERSIZE) {
        mp_printf(&mp_plat_print, "File too large.\n");
    }

    return mp_obj_new_str((char*)source,strlen((char*)source));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_readfile_obj, sdcard_readfile);

// Execute a text file containing Python code from SD Card
STATIC mp_obj_t sdcard_execfile(mp_obj_t filenum_obj) {

    uint8_t filenum = mp_obj_get_int(filenum_obj);

    uint16_t offset=0;
	unsigned char source[BUFFERSIZE];

    long next_cluster=Do_SD_get_first_cluster(filenum);

    do {
        next_cluster=Do_SD_read_file(next_cluster, &source[offset], &offset);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF && offset < BUFFERSIZE);

    if (offset>=BUFFERSIZE) {
        mp_printf(&mp_plat_print, "File too large.\n");
    } else {
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, (char*)source, strlen((char*)source), 0);
            qstr source_name = lex->source_name;
            mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
            mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
            mp_call_function_0(module_fun);
            nlr_pop();
        } else {
            // uncaught exception
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        }
    };
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_execfile_obj, sdcard_execfile);

// Execute a text file containing Python code from SD Card, searching by name
STATIC mp_obj_t sdcard_execfilebyname(const mp_obj_t filename_obj) {
    mp_check_self(mp_obj_is_str_or_bytes(filename_obj));

    GET_STR_DATA_LEN(filename_obj, str, str_len);
    char filename_str[str_len];
    strcpy(filename_str, (char *)str);

    uint8_t filenum = Do_SD_find_file_by_name(filename_str);

    if (filenum<40) {
        uint16_t offset=0;
        unsigned char source[BUFFERSIZE];

        long next_cluster=Do_SD_get_first_cluster(filenum);

        do {
            next_cluster=Do_SD_read_file(next_cluster, &source[offset], &offset);
        } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF && offset < BUFFERSIZE);

        if (offset>=BUFFERSIZE) {
            mp_printf(&mp_plat_print, "File too large.\n");
        } else {
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, (char*)source, strlen((char*)source), 0);
                qstr source_name = lex->source_name;
                mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
                mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
                mp_call_function_0(module_fun);
                nlr_pop();
            } else {
                // uncaught exception
                mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            }
        };
    } else {
        mp_printf(&mp_plat_print, "File not found.\n");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_execfilebyname_obj, sdcard_execfilebyname);

/*  Work in progress

// Execute a mpy file containing Python code from SD Card
STATIC mp_obj_t sdcard_loadfile(mp_obj_t filenum_obj) {

    uint8_t filenum = mp_obj_get_int(filenum_obj);

    uint16_t offset=0;
	unsigned char source[BUFFERSIZE];

    long next_cluster=Do_SD_get_first_cluster(filenum);

    do {
        next_cluster=Do_SD_read_file(next_cluster, &source[offset], &offset);
    } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF && offset < BUFFERSIZE);

    if (offset>=BUFFERSIZE) {
        mp_printf(&mp_plat_print, "File too large.\n");
    } else {
    }
    //mp_obj_t module_obj;

    //mp_raw_code_t *raw_code = mp_raw_code_load_file((char*)source);
    //do_execute_raw_code(module_obj, raw_code, (char*)source);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sdcard_loadfile_obj, sdcard_loadfile);*/

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t sdcard_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_sdcard) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&sdcard_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&sdcard_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_listdir), MP_ROM_PTR(&sdcard_listdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_printfile), MP_ROM_PTR(&sdcard_printfile_obj) },
    { MP_ROM_QSTR(MP_QSTR_printfilebin), MP_ROM_PTR(&sdcard_printfilebin_obj) },
    { MP_ROM_QSTR(MP_QSTR_printfilebyname), MP_ROM_PTR(&sdcard_printfilebyname_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfile), MP_ROM_PTR(&sdcard_readfile_obj) },
    { MP_ROM_QSTR(MP_QSTR_execfile), MP_ROM_PTR(&sdcard_execfile_obj) },
    { MP_ROM_QSTR(MP_QSTR_execfilebyname), MP_ROM_PTR(&sdcard_execfilebyname_obj) },
//    { MP_ROM_QSTR(MP_QSTR_loadfile), MP_ROM_PTR(&sdcard_loadfile_obj) },
};
STATIC MP_DEFINE_CONST_DICT(sdcard_module_globals, sdcard_module_globals_table);

const mp_obj_module_t sdcard_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&sdcard_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sdcard, sdcard_module, MICROPY_MODULE_SDCARD);