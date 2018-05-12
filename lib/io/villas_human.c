/** The internal datastructure for a sample of simulation data.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#include <villas/io.h>
#include <villas/plugin.h>
#include <villas/utils.h>
#include <villas/timing.h>
#include <villas/sample.h>
#include <villas/io/villas_human.h>

size_t villas_human_sprint_single(char *buf, size_t len, struct sample *s, int flags)
{
	size_t off = 0;

	if (flags & SAMPLE_HAS_ORIGIN) {
		off += snprintf(buf + off, len - off, "%llu", (unsigned long long) s->ts.origin.tv_sec);
		off += snprintf(buf + off, len - off, ".%09llu", (unsigned long long) s->ts.origin.tv_nsec);
	}

	if (flags & SAMPLE_HAS_RECEIVED)
		off += snprintf(buf + off, len - off, "%+e", time_delta(&s->ts.origin, &s->ts.received));

	if (flags & SAMPLE_HAS_SEQUENCE)
		off += snprintf(buf + off, len - off, "(%u)", s->sequence);

	if (flags & SAMPLE_HAS_VALUES) {
		for (int i = 0; i < s->length; i++) {
			switch (sample_get_data_format(s, i)) {
				case SAMPLE_DATA_FORMAT_FLOAT:
					off += snprintf(buf + off, len - off, "\t%.6lf", s->data[i].f);
					break;
				case SAMPLE_DATA_FORMAT_INT:
					off += snprintf(buf + off, len - off, "\t%" PRIi64, s->data[i].i);
					break;
			}
		}
	}

	off += snprintf(buf + off, len - off, "\n");

	return off;
}

size_t villas_human_sscan_single(const char *buf, size_t len, struct sample *s, int flags)
{
	char *end;
	const char *ptr = buf;

	double offset = 0;

	s->flags = 0;

	/* Format: Seconds.NanoSeconds+Offset(SequenceNumber) Value1 Value2 ...
	 * RegEx: (\d+(?:\.\d+)?)([-+]\d+(?:\.\d+)?(?:e[+-]?\d+)?)?(?:\((\d+)\))?
	 *
	 * Please note that only the seconds and at least one value are mandatory
	 */

	/* Mandatory: seconds */
	s->ts.origin.tv_sec = (uint32_t) strtoul(ptr, &end, 10);
	if (ptr == end || *end == '\n')
		return -1;

	s->flags |= SAMPLE_HAS_ORIGIN;

	/* Optional: nano seconds */
	if (*end == '.') {
		ptr = end + 1;

		s->ts.origin.tv_nsec = (uint32_t) strtoul(ptr, &end, 10);
		if (ptr == end)
			return -3;
	}
	else
		s->ts.origin.tv_nsec = 0;

	/* Optional: offset / delay */
	if (*end == '+' || *end == '-') {
		ptr = end;

		offset = strtof(ptr, &end); /* offset is ignored for now */
		if (ptr != end)
			s->flags |= SAMPLE_HAS_OFFSET;
		else
			return -4;
	}

	/* Optional: sequence */
	if (*end == '(') {
		ptr = end + 1;

		s->sequence = strtoul(ptr, &end, 10);
		if (ptr != end)
			s->flags |= SAMPLE_HAS_SEQUENCE;
		else
			return -5;

		if (*end == ')')
			end++;
	}

	for (ptr  = end, s->length = 0;
	                 s->length < s->capacity;
	     ptr  = end, s->length++) {
		if (*end == '\n')
			break;

		switch (s->format & (1 << s->length)) {
			case SAMPLE_DATA_FORMAT_FLOAT:
				s->data[s->length].f = strtod(ptr, &end);
				break;
			case SAMPLE_DATA_FORMAT_INT:
				s->data[s->length].i = strtol(ptr, &end, 10);
				break;
		}

		 /* There are no valid FP values anymore. */
		if (end == ptr)
			break;
	}

	if (*end == '\n')
		end++;

	if (s->length > 0)
		s->flags |= SAMPLE_HAS_VALUES;

	if (s->flags & SAMPLE_HAS_OFFSET) {
		struct timespec off = time_from_double(offset);
		s->ts.received = time_add(&s->ts.origin, &off);

		s->flags |= SAMPLE_HAS_RECEIVED;
	}

	return end - buf;
}

int villas_human_sprint(char *buf, size_t len, size_t *wbytes, struct sample *smps[], unsigned cnt, int flags)
{
	int i;
	size_t off = 0;

	for (i = 0; i < cnt && off < len; i++)
		off += villas_human_sprint_single(buf + off, len - off, smps[i], flags);

	if (wbytes)
		*wbytes = off;

	return i;
}

int villas_human_sscan(char *buf, size_t len, size_t *rbytes, struct sample *smps[], unsigned cnt, int flags)
{
	int i;
	size_t off = 0;

	for (i = 0; i < cnt && off < len; i++)
		off += villas_human_sscan_single(buf + off, len - off, smps[i], flags);

	if (rbytes)
		*rbytes = off;

	return i;
}

void villas_human_header(struct io *io)
{
	FILE *f = io_stream_output(io);

	fprintf(f, "# %-20s\t\t%s\n", "sec.nsec+offset", "data[]");
}

static struct plugin p = {
	.name = "villas.human",
	.description = "VILLAS human readable format",
	.type = PLUGIN_TYPE_IO,
	.io = {
		.sprint	= villas_human_sprint,
		.sscan	= villas_human_sscan,
		.header = villas_human_header,
		.size	= 0,
		.flags	= IO_NEWLINES
	}
};

REGISTER_PLUGIN(&p);
