
## REST APIs


Method | URL
---|---
GET | /config
GET | /view
GET | /view/$app-name
PUT | /reg
POST| /start
POST| /stop
DELETE| /unreg




## Show all sub command


```
$ appc
Commands:
  view        List all applications
  config      Display configuration infomration
  start       Start application[s]
  stop        Stop application[s]
  reg         Add a new application
  unreg       Remove an application

Run 'appmgc COMMAND --help' for more information on a command.

Usage:  appmgc [COMMAND] [ARG...] [flags]
```


## Display applications

```
$ appc view
id user  active pid   return name        command_line
--------------------------------------------------------------------
1  root  start  11051 0      TestApp     /bin/sleep 20
--------------------------------------------------------------------

$ appc view --name TestApp
id user  active pid   return name        command_line
--------------------------------------------------------------------
1  root  start  23407 0      TestApp     /bin/sleep 20
--------------------------------------------------------------------
```

## Display configurations

```
$ appc config
--------------------------------------------------------------------
{
   "Applications" : [
      {
         "active" : 1,
         "command_line" : "/bin/sleep 20",
         "daily_limitation" : {
            "daily_end" : "23:00:00",
            "daily_start" : "09:00:00"
         },
         "env" : {
            "TEST_ENV1" : "value",
            "TEST_ENV2" : "value"
         },
         "keep_running" : true,
         "name" : "TestApp",
         "run_as" : "root",
         "start_interval_seconds" : 30,
         "start_interval_timeout" : 0,
         "start_time" : "2018-01-01 16:00:00",
         "working_dir" : "/opt"
      }
   ],
   "HostDescription" : "Host01",
   "RestListenPort" : 6060,
   "ScheduleIntervalSec" : 2
}

--------------------------------------------------------------------
```

## Register a new application

Supported App type|
---|
Long Running application | 
App run periodic |
Long running applicatin but will restarted periodic |
Application can be only avialable in a specific time range daily|
```
appc reg 
Add a new application::
  -n [ --name ] arg              application name
  -u [ --user ] arg              application process running user name
  -c [ --cmd ] arg               full command line with arguments
  -w [ --workdir ] arg (=/tmp)   working directory
  -a [ --active ] arg (=1)       application active status (start is true, stop
                                 is false)
  -t [ --time ] arg              start date time for short running app (e.g., 
                                 '2018-01-01 09:00:00')
  -s [ --daily_start ] arg       daily start time (e.g., '09:00:00')
  -d [ --daily_end ] arg         daily end time (e.g., '20:00:00')
  -e [ --env ] arg               environment variables (e.g., 
                                 env1=value1:env2=value2)
  -i [ --interval ] arg          start interval seconds for short running app
  -x [ --extraTime ] arg         extra timeout for short running app,the value 
                                 must less than interval  (default 0
  -k [ --keep_running ] arg (=0) monitor and keep running for short running app
                                 in start interval
  -f [ --force ]                 force without confirm.
  -h [ --help ]                  produce help message
```


## Remove a application
## Start a application
## Stop a application
