#ifndef _CMD_H
#define _CMD_H

#include "stdint.h"
#include "setup.h"
#include "http.h"

void handle_cmd(Buffer_t * buffer1, PortStates * port1_state, CmdStates * cmd_state, Wifi_t * wifi, Http_t * http);

#endif	/* _CMD_H */
