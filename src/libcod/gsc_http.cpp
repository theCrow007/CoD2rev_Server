#include "gsc_http.hpp"

#include "gsc.hpp"
#include "http_client.hpp"

#include <string.h>

static HttpClient *gsc_http_client;
static int gsc_http_pending_requests;

void gsc_http_fetch()
{
	if (Scr_GetNumParam() != 7)
	{
		stackError("http_fetch: invalid number of parameters, expected 7, got %d", Scr_GetNumParam());
		stackPushUndefined();
		return;
	}

	const char *url;
	const char *method;
	const char *data;
	const char *headers;
	int timeout;
	int onDoneCallback;
	int onErrorCallback;

	if (!stackGetParams("ssssi", &url, &method, &data, &headers, &timeout) ||
		!stackGetParamFunction(5, &onDoneCallback) ||
		!stackGetParamFunction(6, &onErrorCallback))
	{
		stackError("http_fetch: expected (string url, string method, string data, string headers, int timeout, function onDone, function onError)");
		stackPushUndefined();
		return;
	}

	if (!gsc_http_client)
		gsc_http_client = new HttpClient();

	gsc_http_pending_requests++;

	gsc_http_client->request(method, url, data, strlen(data), headers,
		[onDoneCallback](const HttpClient::Response& res)
		{
			gsc_http_pending_requests--;

			if (!onDoneCallback || !Scr_IsSystemActive())
				return;

			stackPushArray();
			for (const auto& header : res.headers)
			{
				stackPushString(header.first.c_str());
				stackPushArrayLast();
				stackPushString(header.second.c_str());
				stackPushArrayLast();
			}

			stackPushString(res.body.c_str());
			stackPushInt(res.status);

			unsigned short thread_id = Scr_ExecThread(onDoneCallback, 3);
			Scr_FreeThread(thread_id);
		},
		[onErrorCallback, url = std::string(url)](const std::string& error)
		{
			gsc_http_pending_requests--;

			if (onErrorCallback && Scr_IsSystemActive())
			{
				stackPushString(error.c_str());
				unsigned short thread_id = Scr_ExecThread(onErrorCallback, 1);
				Scr_FreeThread(thread_id);
				return;
			}

			Com_Printf("HTTP error while fetching %s: %s\n", url.c_str(), error.c_str());
		},
		timeout);
}

void gsc_http_frame()
{
	if (gsc_http_client)
		gsc_http_client->poll();
}

void gsc_http_shutdown()
{
	if (!gsc_http_client)
		return;

	for (int i = 0; i < 10 && gsc_http_pending_requests > 0; i++)
		gsc_http_client->poll(100);

	delete gsc_http_client;
	gsc_http_client = NULL;
	gsc_http_pending_requests = 0;
}

