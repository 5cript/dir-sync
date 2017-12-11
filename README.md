# dir-sync
A rudimentary directory synchronizer and auto copy tool

Icon: https://www.iconfinder.com/icons/61552/arrows_blue_icon#size=128
        License: Free for commercial use
## API
Look into server.cpp

## Program Options
```
Possible options:
  --help                   produce help message
  -p [ --port ] arg        binding port for REST interface
  -s [ --start ]           starts copy operation immediately
  -t [ --tasks ] arg       a persistence file, with saved tasks
  -i [ --interval ] arg    The refresh interval in milliseconds, must be larger
                           than 100ms.                         
  -m [ --scanFileMax ] arg Maximum files to be scanned each round
  ```
