#ifndef GSC_WEBSOCKET_HPP
#define GSC_WEBSOCKET_HPP

void gsc_websocket_connect();
void gsc_websocket_close();
void gsc_websocket_sendText();
bool gsc_websocket_beforeMapChangeOrRestart(bool fromScript, bool bComplete, bool shutdown);
void gsc_websocket_frame();
void gsc_websocket_init();

#endif
