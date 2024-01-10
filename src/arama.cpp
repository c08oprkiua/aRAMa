#include "gecko/gecko_processor.h"
#include "arama.h"

#include <nsysnet/socket.h>
#include <gx2/event.h>

static int runGeckoServer(GeckoProcessor *processor){

    int sockfd = -1, len;
    struct sockaddr_in socketAddress;

	//setup_os_exceptions();
	//socket_lib_init();
	//initializeUDPLog();

    GeckoSetUp = true;

	while (GeckoSetUp) {
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = 7331;
		socketAddress.sin_addr.s_addr = 0;

		log_printf("socket()...\n");
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHECK_ERROR(sockfd == -1);

		log_printf("bind()...\n");
		processor->ret = bind(sockfd, (struct sockaddr *) &socketAddress, (socklen_t) 16);
		CHECK_ERROR(processor->ret < 0);

		log_printf("listen()...\n");
		processor->ret = listen(sockfd, (int) 20);
		CHECK_ERROR(processor->ret < 0);

		while (true) {
			len = 16;
			log_printf("before accept()...\n");
			processor->clientfd = accept(sockfd, (struct sockaddr *) &socketAddress, (socklen_t *) &len);
			log_printf("after accept()...\n");
			CHECK_ERROR(processor->clientfd == -1);
			log_printf("commands()...\n");
			processor->ret = processor->processCommands();
			CHECK_ERROR(processor->ret < 0);
			socketclose(processor->clientfd);
			processor->clientfd = -1;

			log_printf("GX2WaitForVsync() inner...\n");
			GX2WaitForVsync();
		}

		error:
		log_printf("error, closing connection...\n");
		if (processor->clientfd != -1)
			socketclose(processor->clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		processor->error = processor->ret;

		// Fix the console freezing when e.g. going to the friend list
		log_printf("GX2WaitForVsync() outer...\n");
		GX2WaitForVsync();
	}
	return 0;
}

static s32 startTCPGeckoThread(s32 argc, void *argv) {
	log_print("Starting TCP Gecko thread...\n");

	// Run the TCP Gecko Installer server
	GeckoProcessor geck_proc;

    /*
	bss = (struct pygecko_bss_t *) memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss == 0)
		return (s32) 0;
	memset(bss, 0, sizeof(struct pygecko_bss_t));
    */

	if (OSCreateThread(geck_proc.thread, runGeckoServer, (uint32_t) geck_proc,
					   geck_proc.stack + sizeof(geck_proc.stack), sizeof(geck_proc.stack), 0, 0xc) == 1) {
		OSResumeThread(bss->thread);
	}

	log_print("TCP Gecko thread started...\n");

	// Execute the code handler if it is installed
	if (isCodeHandlerInstalled) {
		log_print("Code handler installed...\n");
		void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

		while (true) {
			usleep(9000);

			// considerApplyingSDCheats();
			// log_print("Running code handler...\n");
			codeHandlerFunction();
			// log_print("Code handler done executing...\n");

			if (assemblySize > 0) {
				executeAssembly();
			}

			if (arama_settings && ARAMA_SET_SD_CODES_ACTIVE) {
				considerApplyingSDCheats();
			}
		}
	} else {
		log_print("Code handler not installed...\n");
	}

	return (s32) 0;
}