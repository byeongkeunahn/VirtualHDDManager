
#pragma once

typedef enum
{
	TOKE,
	TOKEN_1

} vhms_token_type;

typedef struct _vhms_token
{
	vhms_token_type	token_type;
	vhmsize_t		orig_off; // Token�� ���� �����Ϳ����� ������
	vhmsize_t		orig_len; // Token�� ���� �����Ϳ����� ����
	vhmsize_t		data_off; // Token�� ó���� �����Ϳ����� ������
	vhmsize_t		data_len; // Token�� ó���� �����Ϳ����� ����

} VHMS_TOKEN;
