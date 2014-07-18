/** Some basic tests.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014, Institute for Automation of Complex Power Systems, EONERC
 */

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

#include "config.h"
#include "msg.h"
#include "node.h"
#include "utils.h"

int sd;

void quit(int sig, siginfo_t *si, void *ptr)
{
	close(sd);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("Usage: %s TEST LOCAL REMOTE\n", argv[0]);
		printf("  TEST     has to be 'latency' for now\n");
		printf("  LOCAL    is a IP:PORT combination of the local host\n");
		printf("  REMOTE   is a IP:PORT combination of the remote host\n\n");
		printf("s2ss Simulator2Simulator Server v%s\n", VERSION);
		printf("Copyright 2014, Institute for Automation of Complex Power Systems, EONERC\n");
		exit(EXIT_FAILURE);
	}

	struct node n = NODE_INIT("remote");

	/* Setup signals */
	struct sigaction sa_quit = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = quit
	};

	sigemptyset(&sa_quit.sa_mask);
	sigaction(SIGTERM, &sa_quit, NULL);
	sigaction(SIGINT, &sa_quit, NULL);

	/* Resolve addresses */
	if (resolve_addr(argv[2], &n.local, 0))
		error("Failed to resolve local address: %s", argv[2]);

	if (resolve_addr(argv[3], &n.remote, 0))
		error("Failed to resolve remote address: %s", argv[3]);

	node_connect(&n);

	debug(1, "We listen at %s:%u", inet_ntoa(n.local.sin_addr), ntohs(n.local.sin_port));
	debug(1, "We sent to %s:%u", inet_ntoa(n.remote.sin_addr), ntohs(n.remote.sin_port));

	if (!strcmp(argv[1], "latency")) {
		struct msg m = MSG_INIT(sizeof(struct timespec) / sizeof(float));
		struct timespec *ts1 = (struct timespec *) &m.data;
		struct timespec *ts2 = malloc(sizeof(struct timespec));

		double rtt, rtt_max = LLONG_MIN, rtt_min = LLONG_MAX, avg = 0;
		int run = 0;


#if 1		/* Print header */
		fprintf(stdout, "%17s", "timestamp");
#endif
		fprintf(stdout, "%10s%10s%10s%10s\n", "rtt", "min", "max", "avg");

		while (1) {
			clock_gettime(CLOCK_REALTIME, ts1);
			msg_send(&m, &n);
			msg_recv(&m, &n);
			clock_gettime(CLOCK_REALTIME, ts2);

			rtt = timespec_delta(ts1, ts2);

			if (rtt < 0) continue;
			if (rtt > rtt_max) rtt_max = rtt;
			if (rtt < rtt_min) rtt_min = rtt;

			avg += rtt;
			run++;

#if 1
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			fprintf(stdout, "%17.6f", ts.tv_sec + ts.tv_nsec / 1e9);
#endif

			fprintf(stdout, "%10.3f%10.3f%10.3f%10.3f\n", 1e3 * rtt, 1e3 * rtt_min, 1e3 * rtt_max, 1e3 * avg / run);

			m.sequence++;
		}

		free(ts2);
	}

	return 0;
}
