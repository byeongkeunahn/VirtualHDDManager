
#pragma once

typedef enum
{
	TOKE,
	TOKEN_1

} vhms_token_type;

typedef struct _vhms_token
{
	vhms_token_type	token_type;
	vhmsize_t		orig_off; // Token의 원본 데이터에서의 오프셋
	vhmsize_t		orig_len; // Token의 원본 데이터에서의 길이
	vhmsize_t		data_off; // Token의 처리된 데이터에서의 오프셋
	vhmsize_t		data_len; // Token의 처리된 데이터에서의 길이

} VHMS_TOKEN;
