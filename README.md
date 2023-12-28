## 1 安装 tinyxml 

把tinyxml的makefile文件中105行，就是Output注释下的LD改为AR指定生成静态库就可以了

或

${OUTPUT}: ${OBJS}
${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
替换成
${OUTPUT}: ${OBJS}
	${AR} $@ ${OBJS}
	${RANLIB} $@
应该就可以了


## cmake 的gdb调试

task.json
```json
{
	"version": "2.0.0",
	"tasks": [
        {
            "label": "cmake",
            "type": "shell",
            "command": "cmake",
            "args": [
                "../"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build"
            },            
        },
        {
            "label": "make",
            "type": "shell",
            "command": "make",
            "args": [],
            "options": {
                "cwd": "${workspaceFolder}/build"
            }, 
        },
        {
            "label": "build",
            "dependsOn":["cmake", "make"]
        },
    ],
}

```

launch.json
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "g++ - Build and debug active file",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/test_tcp",
            "args": ["para1", "para2"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}


```

## 3 pthread_mutex_init
$ man pthread_mutex_init
No manual entry for pthread_mutex_init
$sudo apt-get install glibc-doc  
即可正常使用 man pthread_mutex_init

