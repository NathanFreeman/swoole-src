# Bug reports

## Instruction

If you think you have found a bug in Swoole, please report it.
The Swoole developers probably don't know about it,
and unless you report it, chances are it won't be fixed.
You can report bugs at https://github.com/swoole/swoole-src/issues.
Please do not send bug reports in the mailing list or personal letters.
The issue page is also suitable to submit feature requests.

Please read the **How to report a bug document** before submitting any bug reports.

## New issue

First, while creating an issue, the system will give the following template:

```markdown
Please answer these questions before submitting your issue. Thanks!
1. What did you do? If possible, provide a simple script for reproducing the error.
2. What did you expect to see?
3. What did you see instead?
4. What version of Swoole are you using (`php --ri swoole`)?
5. What is your machine environment used (including the version of kernel & php & gcc)?
```
The most important thing is to provide a simple script for reproducing the error, otherwise, you must provide as much information as possible.

## Memory detection (recommended)

In addition to using `gdb` analysis, you can use the `valgrind` tool to check if the program is working properly.

```shell
USE_ZEND_ALLOC=0 valgrind --log-file=/tmp/valgrind.log php your_file.php
```

* After the program is executed to the wrong location, `ctrl+c` is interrupted, and upload the `/tmp/valgrind.log` file.

## Address Sanitizer
In certain cases, crash issues only occur in complex production environments, not consistently reproducible, and with a very low probability of occurrence. The use of `Valgrind` significantly impacts performance, whereas `Address Sanitizer` allows for memory detection without affecting performance.

We have created a `debug` version of `Swoole`, which can be compiled with the `--enable-debug` and `--enable-address-sanitizer` parameters to enable `Address Sanitizer`. The usage is as follows:

```shell
docker pull phpswoole/swoole:dev-debug
docker run -it --rm phpswoole/swoole:dev-debug /bin/bash
php your_file.php
```

> To use `ASAN`, be sure to disable Address Space Layout Randomization (`ASLR`):

```shell
echo 0 > /proc/sys/kernel/randomize_va_space
```

This docker image can be used directly in a production environment, and `Address Sanitizer` will print error messages and call stack information when memory errors occur. This information can assist developers in quickly pinpointing issues.

## CoreDump

Besides, In a special case, you can use debugging tools to help developers locate problems

```shell
WARNING	Worker::report_error(): worker(pid=%d, id=%d) abnormal exit, status=0, signal=11
```

When a segmentation error occurs with Swoole, You can use the `gdb` tool and use `bt` command.
> Using `gdb` to track the core file need to add the `--enable-debug` parameter when compiling `swoole`.

Enable core dump
```shell
ulimit -c unlimited
```

Use `gdb` to view the `core dump` information. The `core` file is usually in the current directory. If the operating system does the processing, put the `core dump` file in another directory, please replace it with the corresponding path.
```
gdb php core
gdb php /tmp/core.4596
```

Enter bt under gdb to view the call stack information.
```
(gdb) bt
```
Use the f command in gdb to view the code segment corresponding to the ID.
```
(gdb)f 1
(gdb)f 0
```

If there is no function call stack information, it may be that the compiler has removed the debug information. Please manually modify the `Makefile` file in the swoole source directory and modify CFLAGS to

```shell
CFLAGS = -Wall -pthread -g -O0
```
