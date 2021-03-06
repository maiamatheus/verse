/*
 *
 * ***** BEGIN BSD LICENSE BLOCK *****
 *
 * Copyright (c) 2009-2011, Jiri Hnidek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END BSD LICENSE BLOCK *****
 *
 * Authors: Jiri Hnidek <jiri.hnidek@tul.cz>
 *
 */

#include <check.h>

#include "v_common.h"
#include "v_commands.h"
#include "v_sys_commands.h"
#include "v_in_queue.h"
#include "v_out_queue.h"


#define TEST_VEC_SIZE	1


#if 0
/**
 * Structure holding test values of negotiation command
 */
typedef struct NEG_cmd_values {
	uint8	opcode;
	uint8	length;
	uint8	feature;
	uint8	values[256];
} NEG_cmd_values;

static struct NEG_cmd_values cmd_values[TEST_VEC_SIZE] = {
	{
		CMD_CHANGE_L_ID,
		4,
		FTR_FC_ID,
		{
			FC_NONE, 0
		}
	}
};
#endif


/**
 * \brief Test adding wrong negotiate command to the list of system commands
 */
START_TEST ( test_add_wrong_negotiate_cmd )
{
	union VSystemCommands sys_cmd[1];
	uint8 cmd_op_code;
	uint8 ftr_op_code;
	uint8 value = 0;
	int ret;

	/* Wrong command OpCode */
	cmd_op_code = CMD_RESERVED_ID;
	ftr_op_code = FTR_RSV_ID;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &value, NULL);

	fail_unless( ret != 1,
			"Adding wrong negotiate command not failed.");

	/* Wrong feature OpCode */
	cmd_op_code = CMD_CHANGE_L_ID;
	ftr_op_code = FTR_RSV_ID;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &value, NULL);

	fail_unless( ret != 1,
			"Adding wrong negotiate command not failed.");

}
END_TEST


/**
 * \brief Test unpacking of corrupted negotiate command
 */
START_TEST ( test_unpack_wrong_negotiate_cmd )
{
	union VSystemCommands recv_sys_cmd[1];
	char buffer[255];
	int buffer_size, cmd_len;

	/* Create buffer with corrupted negotiate command (wrong command length) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 0;					/* Wrong Command Length */
	buffer_size = 2;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	/* Create buffer with corrupted negotiate command (small buffer size) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 4;					/* Command Length */
	buffer[2] = FTR_CC_ID;			/* Feature (Congestion) */
	buffer[3] = CC_TCP_LIKE;		/* Type of congestion */
	buffer_size = 3;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_size,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_size, cmd_len);

	/* Create buffer with corrupted negotiate command
	 * (wrong command feature number) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 3;					/* Command Length */
	buffer[2] = FTR_RSV_ID;			/* Wrong Command Feature  */
	buffer_size = 3;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_size,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_size, cmd_len);

	/* Create buffer with corrupted negotiate command
	 * (wrong command feature number) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 3;					/* Command Length */
	buffer[2] = 100;				/* Wrong Command Feature */
	buffer_size = 3;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_size,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_size, cmd_len);

	/* Create buffer with corrupted negotiate command
	 * (wrong size of negotiated string) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 8;					/* Command Length */
	buffer[2] = FTR_CLIENT_NAME;	/* Command Feature */
	buffer[3] = 5;					/* Wrong String length */
	buffer[4] = 'a';				/* String ... */
	buffer[4] = 'h';
	buffer[4] = 'o';
	buffer[4] = 'y';
	buffer_size = 8;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_size,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_size, cmd_len);

	/* Create buffer with corrupted negotiate command
	 * (wrong size of command including negotiated  string) */
	buffer[0] = CMD_CHANGE_L_ID;	/* Command OpCode */
	buffer[1] = 7;					/* Command Length */
	buffer[2] = FTR_CLIENT_NAME;	/* Command Feature */
	buffer[3] = 4;					/* Wrong String length */
	buffer[4] = 'a';				/* String ... */
	buffer[4] = 'h';
	buffer[4] = 'o';
	buffer[4] = 'y';
	buffer_size = 8;

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_size,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == 7,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_size, cmd_len);
}
END_TEST


/**
 * \brief Test simple adding negotiate command to the list of system commands
 */
START_TEST ( test_add_negotiate_cmd_single_value )
{
	union VSystemCommands sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_L_ID;
	uint8 ftr_op_code = FTR_FC_ID;
	uint8 value = FC_NONE;
	int ret;

	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &value, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");
	fail_unless( sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			sys_cmd->negotiate_cmd.count, 1);
	fail_unless( sys_cmd->negotiate_cmd.value[0].uint8 == value,
			"Negotiate command value: %d != %d",
			sys_cmd->negotiate_cmd.value[0].uint8, value);

}
END_TEST


/**
 * \brief Test simple adding negotiate command to the list of system commands
 */
START_TEST ( test_print_negotiate_cmds )
{
	union VSystemCommands sys_cmd[1];
	uint8 cmd_op_code;
	uint8 ftr_op_code;
	uint8 uint8_value = 1;
	real32 real32_value = 60.0f;
	char string[5] = {'a', 'h', 'o', 'y', '\0'};
	int ret;

	cmd_op_code = CMD_CHANGE_R_ID;
	ftr_op_code = FTR_FC_ID;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &uint8_value, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_CC_ID;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &uint8_value, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_R_ID;
	ftr_op_code = FTR_HOST_URL;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &string, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CHANGE_L_ID;
	ftr_op_code = FTR_TOKEN;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &string, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_DED;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &string, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_RWIN_SCALE;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &uint8_value, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_FPS;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &real32_value, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_CMD_COMPRESS;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &uint8_value, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_CLIENT_NAME;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &string, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);

	cmd_op_code = CMD_CONFIRM_L_ID;
	ftr_op_code = FTR_CLIENT_VERSION;
	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, &string, NULL);
	fail_unless( ret == 1,
			"Adding negotiate command failed");
	v_print_negotiate_cmd(VRS_PRINT_NONE, &sys_cmd->negotiate_cmd);
}
END_TEST


/**
 * \brief Test adding negotiate command with string value
 * to the list of system commands.
 */
START_TEST ( test_add_negotiate_cmd_string_value )
{
	union VSystemCommands sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_L_ID;
	uint8 ftr_op_code = FTR_CLIENT_NAME;
	char string[5] = {'a', 'h', 'o', 'y', '\0'};
	int ret;

	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code, string, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");
	fail_unless( sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			sys_cmd->negotiate_cmd.count, 1);
	fail_unless( sys_cmd->negotiate_cmd.value[0].string8.length == 4,
			"Negotiate command value (string length): %d != %d",
			sys_cmd->negotiate_cmd.value[0].string8.length, 4);
	fail_unless( strcmp((char*)sys_cmd->negotiate_cmd.value[0].string8.str, "ahoy") == 0,
			"Negotiate command value (string): %s != ahoy",
			sys_cmd->negotiate_cmd.value[0].string8.str);

}
END_TEST

/**
 * \brief Test of adding negotiate command with multiple values
 * to the list of system commands
 */
START_TEST ( test_add_negotiate_cmd_multiple_values )
{
	union VSystemCommands sys_cmd[2];
	uint8 cmd_op_code = CMD_CHANGE_L_ID;
	uint8 ftr_op_code = FTR_FC_ID;
	uint8 values[2] = {FC_TCP_LIKE, FC_NONE};
	int ret;

	ret = v_add_negotiate_cmd(sys_cmd, 0, cmd_op_code, ftr_op_code,
			&values[0], &values[1], NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");
	fail_unless( sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( sys_cmd->negotiate_cmd.count == 2,
			"Negotiate command feature count: %d != %d",
			sys_cmd->negotiate_cmd.count, 2);
	fail_unless( sys_cmd->negotiate_cmd.value[0].uint8 == values[0],
			"Negotiate command value[0]: %d != %d",
			sys_cmd->negotiate_cmd.value[0].uint8, values[0]);
	fail_unless( sys_cmd->negotiate_cmd.value[1].uint8 == values[1],
			"Negotiate command value[1]: %d != %d",
			sys_cmd->negotiate_cmd.value[1].uint8, values[1]);

}
END_TEST


/**
 * \brief Test simple packing and unpacking of negotiate command
 */
START_TEST ( test_pack_unpack_negotiate_cmd_single_value )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_L_ID;
	uint8 ftr_op_code = FTR_FC_ID;
	uint8 value = FC_NONE;
	char buffer[255];
	int ret, buffer_pos = 0, cmd_len;

	/* Create negotiate command */
	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code,
			&value, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 4,
			"Length of packed cmd: %d != %d",
			buffer_pos, 4);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 1);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].uint8 == value,
			"Negotiate command value: %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].uint8, value);
}
END_TEST


/**
 * \brief Test simple packing and unpacking of negotiate command
 */
START_TEST ( test_pack_unpack_negotiate_cmd_float_value )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_L_ID;
	uint8 ftr_op_code = FTR_FPS;
	real32 value = 60.0f;
	char buffer[255];
	int ret, buffer_pos = 0, cmd_len;

	/* Create negotiate command */
	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code,
			&value, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 7,
			"Length of packed cmd: %d != %d",
			buffer_pos, 7);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 1);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].real32 == value,
			"Negotiate command value: %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].real32, value);
}
END_TEST


/**
 * \brief Test of packing and unpacking negotiate command with string value.
 */
START_TEST ( test_pack_unpack_negotiate_cmd_string_value )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_R_ID;
	uint8 ftr_op_code = FTR_CLIENT_NAME;
	char string[5] = {'a', 'h', 'o', 'y', '\0'};
	char buffer[255];
	int ret, buffer_pos = 0, cmd_len;

	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code, string, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 8,
			"Length of packed cmd: %d != %d",
			buffer_pos, 8);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 1);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].string8.length == 4,
			"Negotiate command value (string length): %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].string8.length, 4);
	fail_unless( strcmp((char*)recv_sys_cmd->negotiate_cmd.value[0].string8.str, "ahoy") == 0,
			"Negotiate command value (string): %s != ahoy",
			recv_sys_cmd->negotiate_cmd.value[0].string8.str);

}
END_TEST


/**
 * \brief Test of packing and unpacking negotiate command with string value.
 */
START_TEST ( test_pack_unpack_negotiate_cmd_long_string_value )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CHANGE_R_ID;
	uint8 ftr_op_code = FTR_CLIENT_NAME;
	char string[470] = "Lorem ipsum dolor sit amet, consectetuer adipiscing "
			"elit. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
			"laboris nisi ut aliquip ex ea commodo consequat. Nullam at arcu "
			"a est sollicitudin euismod. Fusce consectetuer risus a nunc. "
			"Cras pede libero, dapibus nec, pretium sit amet, tempor quis. "
			"Etiam dictum tincidunt diam. Nullam lectus justo, vulputate "
			"eget mollis sed, tempor sed magna. Vivamus porttitor turpis ac "
			"leo. Suspendisse sagittis ultrices augue.";
	char buffer[512];
	int ret, buffer_pos = 0, cmd_len;

	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code, string, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 261,
			"Length of packed cmd: %d != %d",
			buffer_pos, 8);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 1,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 1);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].string8.length == 255,
			"Negotiate command value (string length): %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].string8.length, 255);
	/* Make original string 255 bytes long */
	string[255] = '\0';
	fail_unless( strncmp((char*)recv_sys_cmd->negotiate_cmd.value[0].string8.str, string, 255) == 0,
			"Negotiate command value (string): %s != %s",
			recv_sys_cmd->negotiate_cmd.value[0].string8.str,
			string);

}
END_TEST


/**
 * \brief Test packing and unpacking of negotiate command with multiple values
 */
START_TEST ( test_pack_unpack_negotiate_cmd_multiple_values )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CONFIRM_L_ID;
	uint8 ftr_op_code = FTR_FC_ID;
	uint8 values[2] = {FC_TCP_LIKE, FC_NONE};
	char buffer[255];
	int ret, buffer_pos = 0, cmd_len;

	/* Create negotiate command */
	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code,
			&values[0], &values[1], NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 5,
			"Length of packed cmd: %d != %d",
			buffer_pos, 5);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 2,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 2);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].uint8 == values[0],
			"Negotiate command value[0]: %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].uint8, values[0]);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[1].uint8 == values[1],
			"Negotiate command value[1]: %d != %d",
			recv_sys_cmd->negotiate_cmd.value[1].uint8, values[1]);
}
END_TEST


/**
 * \brief Test of packing and unpacking negotiate command with
 * two string values.
 */
START_TEST ( test_pack_unpack_negotiate_cmd_multiple_string_values )
{
	union VSystemCommands send_sys_cmd[1], recv_sys_cmd[1];
	uint8 cmd_op_code = CMD_CONFIRM_R_ID;
	uint8 ftr_op_code = FTR_CLIENT_NAME;
	char string_val0[5] = {'a', 'h', 'o', 'y', '\0'};
	char string_val1[4] = {'h', 'e', 'y', '\0'};
	char buffer[255];
	int ret, buffer_pos = 0, cmd_len;

	ret = v_add_negotiate_cmd(send_sys_cmd, 0, cmd_op_code, ftr_op_code,
			string_val0, string_val1, NULL);

	fail_unless( ret == 1,
			"Adding negotiate command failed");

	/* Pack negotiate command */
	buffer_pos += v_raw_pack_negotiate_cmd(buffer,
			&send_sys_cmd[0].negotiate_cmd);

	fail_unless( buffer_pos == 12,
			"Length of packed cmd: %d != %d",
			buffer_pos, 12);

	/* Unpack system command */
	cmd_len = v_raw_unpack_negotiate_cmd(buffer, buffer_pos,
			&recv_sys_cmd[0].negotiate_cmd);

	fail_unless( cmd_len == buffer_pos,
			"Length of packed and unpacked cmd: %d != %d",
			buffer_pos, cmd_len);
	fail_unless( recv_sys_cmd->negotiate_cmd.id == cmd_op_code,
			"Negotiate command OpCode: %d != %d",
			recv_sys_cmd->negotiate_cmd.id, cmd_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.feature == ftr_op_code,
			"Negotiate command feature: %d != %d",
			recv_sys_cmd->negotiate_cmd.feature, ftr_op_code);
	fail_unless( recv_sys_cmd->negotiate_cmd.count == 2,
			"Negotiate command feature count: %d != %d",
			recv_sys_cmd->negotiate_cmd.count, 2);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[0].string8.length == 4,
			"Negotiate command value (string length): %d != %d",
			recv_sys_cmd->negotiate_cmd.value[0].string8.length, 4);
	fail_unless( strcmp((char*)recv_sys_cmd->negotiate_cmd.value[0].string8.str, "ahoy") == 0,
			"Negotiate command value (string): %s != ahoy",
			recv_sys_cmd->negotiate_cmd.value[0].string8.str);
	fail_unless( recv_sys_cmd->negotiate_cmd.value[1].string8.length == 3,
			"Negotiate command value (string length): %d != %d",
			recv_sys_cmd->negotiate_cmd.value[1].string8.length, 3);
	fail_unless( strcmp((char*)recv_sys_cmd->negotiate_cmd.value[1].string8.str, "hey") == 0,
			"Negotiate command value (string): %s != hey",
			recv_sys_cmd->negotiate_cmd.value[1].string8.str);

}
END_TEST


/**
 * \brief This function creates test suite for Node_Create command
 */
struct Suite *negotiate_suite(void)
{
	struct Suite *suite = suite_create("Negotiate_Cmd");
	struct TCase *tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_add_wrong_negotiate_cmd);
	tcase_add_test(tc_core, test_unpack_wrong_negotiate_cmd);
	tcase_add_test(tc_core, test_add_negotiate_cmd_single_value);
	tcase_add_test(tc_core, test_print_negotiate_cmds);
	tcase_add_test(tc_core, test_add_negotiate_cmd_string_value);
	tcase_add_test(tc_core, test_add_negotiate_cmd_multiple_values);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_single_value);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_float_value);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_string_value);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_long_string_value);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_multiple_values);
	tcase_add_test(tc_core, test_pack_unpack_negotiate_cmd_multiple_string_values);

	suite_add_tcase(suite, tc_core);

	return suite;
}


