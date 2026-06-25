--TEST--
swoole_process: invalid path
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
use Swoole\Process;
$process = new Process(function(Process $process) {
    $process->exec("", []);
}, false);

$process->start();
Process::wait();
?>
--EXPECTF--
Fatal error: Uncaught ValueError: Swoole\Process::exec(): Argument #1 ($exec_file) must not contain any null bytes in %s:%d
Stack trace:
#0 %s
#1 [internal function]: %s
#2 %s
#3 {main}
  thrown in %s on line %d
