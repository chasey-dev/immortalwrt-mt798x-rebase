#ifndef RUIJIE_WEXT_COMPAT_H
#define RUIJIE_WEXT_COMPAT_H

#include <net/iw_handler.h>

/* The target RAM kernel does not export the legacy WEXT stream writers. */
static inline char *ruijie_iwe_stream_add_event(struct iw_request_info *info,
		char *stream, char *end, struct iw_event *event, int length)
{
	int header = iwe_stream_lcp_len(info);

	length = iwe_stream_event_len_adjust(info, length);
	if (length < header || end - stream <= length)
		return stream;

	event->len = length;
	memcpy(stream, event, IW_EV_LCP_PK_LEN);
	memcpy(stream + header, &event->u, length - header);
	return stream + length;
}

static inline char *ruijie_iwe_stream_add_point(struct iw_request_info *info,
		char *stream, char *end, struct iw_event *event, const char *extra)
{
	int header = iwe_stream_lcp_len(info);
	int point = iwe_stream_point_len(info);
	int data_length = event->u.data.length;
	int length = point + data_length;

	if (end - stream <= length)
		return stream;

	event->len = length;
	memcpy(stream, event, IW_EV_LCP_PK_LEN);
	memcpy(stream + header, (const char *)&event->u + IW_EV_POINT_OFF,
	       IW_EV_POINT_PK_LEN - IW_EV_LCP_PK_LEN);
	if (data_length && extra)
		memcpy(stream + point, extra, data_length);
	return stream + length;
}

#endif
