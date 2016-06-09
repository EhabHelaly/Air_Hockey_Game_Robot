Air_Hockey_Game.exe COM9  175 050 036 095 255 250 110 096 091 130 240 250  0 0 0 30
::                  Port | L   H | L   H | L   H | L   H | L   H | L   H | P M C F 
::		    	 |---h---|---s---|---v---|---h---|---s---|---v---|
::		   	 |----------puck---------|---------pusher--------|
::
:: Port >> serial port id where data are sent to
:: P 	>> configuration flag (default is  0) >> 0-: use default configurations
::					         1+: load configuration from configuration.ini
:: M 	>> game modes 	      (default is  0) >> 0-:game (robot vs human) 
::				     		 1 :simulation (robot vs virtual puck)
::				     		 2+:simulation (virtual puck and robot)
:: C 	>> camera id 	      (default is  0)
:: F 	>> output video fps   (default is 60)

