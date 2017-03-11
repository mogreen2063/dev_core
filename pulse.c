      /* send_msg_ram(http_content_length,1); */
      /* send_msg(crlf,1); */

      /* http_packet_length = http_get_packet_length(0); */
      /* hex2dec(http_content_length,http_packet_length); */
      /* send_msg_ram(http_content_length,1); */
      /* send_msg(crlf,1); */
      
      /* while(1); */
      
      /* implement WDT */
      /* switch(pulse_state) { */
      /* case(PULSE_INIT): */
      /* 	if(!PORTBbits.RB0) { */
      /* 	  INTCON3bits.INT1IF = 0; */
      /* 	  INTCON2bits.INTEDG1 = 1; */
      /* 	  INTCON3bits.INT1IE = 1; */
      /* 	  time_flag = 0; */
      /* 	  time_sum = 0; */
      /* 	  pulse_count = 16; */
      /* 	  pulse_state = PULSE_ON; */
      /* 	  TMR0H = 0;		/\* reset timer0 *\/ */
      /* 	  TMR0L = 0; */
      /* 	  LATCbits.LATC2 = 1;	 /\* start pulse *\/ */
      /* 	  Delay10TCYx(6); */
      /* 	  LATCbits.LATC2 = 0; */
      /* 	} */
      /* 	break; */
      /* case(PULSE_ON): */
      /* 	if(pulse_count) */
      /* 	  { */
      /* 	    if(time_flag) { */
      /* 	      pulse_count--; */
      /* 	      time = TMR0L;		/\* read timer *\/ */
      /* 	      time = TMR0H; */
      /* 	      time = time << 8; */
      /* 	      time |= TMR0L; */

      /* 	      get_time(time_string); */
      /* 	      send_msg_ram(time_string,1); */
      /* 	      send_msg(wifi_cmds[0],1); */

      /* 	      time_sum = time_sum + ((uint32_t)time); */
      /* 	      time_flag = 0; */
      /* 	      TMR0H = 0;		/\* reset timer0 *\/ */
      /* 	      TMR0L = 0; */
      /* 	      LATCbits.LATC2 = 1;	 /\* start pulse *\/ */
      /* 	      Delay10TCYx(6); */
      /* 	      LATCbits.LATC2 = 0; */
      /* 	      Delay10KTCYx(60); */
      /* 	    } */
      /* 	  } */
      /* 	else */
      /* 	  { */
      /* 	    time_sum = time_sum >> 4; */
      /* 	    time = time_sum; */
      /* 	    get_time(time_string); */
      /* 	    send_msg(wifi_cmds[0],1); */
      /* 	    send_msg_ram(time_string,1); */
      /* 	    send_msg(wifi_cmds[0],1); */
      /* 	    pulse_state = PULSE_OFF; */
      /* 	  } */
      /* 	break; */
      /* case(PULSE_OFF): */
      /* 	/\* enter sleep *\/ */
      /* 	Delay10KTCYx(255); */
      /* 	pulse_state = PULSE_INIT; */
      /* 	break; */
      /* default: */
      /* 	break; */
      /* } */

      /* time++; */
      /* get_time(time_string); */
      
