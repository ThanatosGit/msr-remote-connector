#include "soc.h"
#include "svc.h"
#include <sys/iosupport.h>
#include "socket.h"
#include <string.h>

uint32_t socMemhandle;
uint32_t SOCU_handle;

// pointers to the original binary
uint32_t* srvHandle =(uint32_t*)0x0076ce08;
void*(*getThreadLocalStorage)(void) = (void*(*)(void)) 0x002fd728;
int32_t(*svcSendSyncRequest)(uint32_t) = (int32_t(*)(uint32_t)) 0x002fd750;

static inline uint32_t IPC_MakeHeader(uint16_t command_id, unsigned normal_params, unsigned translate_params) {
	return ((uint32_t) command_id << 16) | (((uint32_t) normal_params & 0x3F) << 6) | (((uint32_t) translate_params & 0x3F) << 0);
}

static inline uint32_t* getThreadCommandBuffer(void) {
	return (uint32_t*)((uint8_t*)getThreadLocalStorage() + 0x80);
}

static inline uint32_t* getThreadStaticBuffers(void) {
	return (uint32_t*)((uint8_t*)getThreadLocalStorage() + 0x180);
}

static inline uint32_t IPC_Desc_CurProcessId(void) {
	return 0x20;
}

int32_t get_service_handle(uint32_t* in_out, const char* service_name) {
	int32_t result_code;
	uint32_t service_handle;

	uint32_t *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(5,4,0);
	strncpy((char*) &cmdbuf[1], service_name,8);
	cmdbuf[3] = strnlen(service_name, 8);;
	cmdbuf[4] = 0x0;
	result_code = svcSendSyncRequest(*srvHandle);

	service_handle = cmdbuf[3];
	*in_out = service_handle;
	result_code = cmdbuf[1];
	return result_code;

}

int32_t SOCU_Initialize(uint32_t memhandle, uint32_t memsize) {
	uint32_t *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,4); // 0x10044
	cmdbuf[1] = memsize;
	cmdbuf[2] = IPC_Desc_CurProcessId();
	cmdbuf[4] = 0;
	cmdbuf[5] = memhandle;

	svcSendSyncRequest(SOCU_handle);

	return cmdbuf[1];
}

uint32_t socInit(uint32_t* context_addr, uint32_t context_size) {
	uint32_t ret = 0;

	ret = svcCreateMemoryBlock(&socMemhandle, (uint32_t)context_addr, context_size, 3, 3);
	if (ret != 0) return ret;

	get_service_handle(&SOCU_handle, "soc:U");
	SOCU_Initialize(socMemhandle, context_size);

	return 0;
}


int socket(int domain, int type, int protocol) {
	uint32_t *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,3,2); // 0x200C2
	cmdbuf[1] = 2;
	cmdbuf[2] = 1;
	cmdbuf[3] = 0;
	cmdbuf[4] = IPC_Desc_CurProcessId();

	svcSendSyncRequest(SOCU_handle);
	return cmdbuf[2];
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	uint8_t tmpaddr[0x1c];
	memset(tmpaddr, 0, 0x1c);

	tmpaddr[0] = 8;
	tmpaddr[1] = addr->sa_family;
	memcpy(&tmpaddr[2], &addr->sa_data, 6);


	uint32_t *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,2,4); // 0x50084
	cmdbuf[1] = (uint32_t)sockfd;
	cmdbuf[2] = (uint32_t)8;
	cmdbuf[3] = IPC_Desc_CurProcessId();
	cmdbuf[5] = (8 << 14) | 2;
	cmdbuf[6] = (uint32_t)tmpaddr;

	svcSendSyncRequest(SOCU_handle);
	return 0;
}

int listen(int sockfd, int max_connections) {
	uint32_t *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,2); // 0x30082
	cmdbuf[1] = (uint32_t)sockfd;
	cmdbuf[2] = (uint32_t)max_connections;
	cmdbuf[3] = IPC_Desc_CurProcessId();

	svcSendSyncRequest(SOCU_handle);
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int tmp_addrlen = 0x1c;
	uint8_t tmpaddr[0x1c];

	memset(tmpaddr, 0, 0x1c);

	uint32_t *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,2,2); // 0x40082
	cmdbuf[1] = (uint32_t)sockfd;
	cmdbuf[2] = (uint32_t)tmp_addrlen;
	cmdbuf[3] = IPC_Desc_CurProcessId();

	uint32_t * staticbufs = getThreadStaticBuffers();
	staticbufs[0] = (tmp_addrlen << 14) | 2;
	staticbufs[1] = (uint32_t)tmpaddr;

	svcSendSyncRequest(SOCU_handle);
	return cmdbuf[2];
}

// TODO: If it ever works implement recv and send
ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return 0;
}
