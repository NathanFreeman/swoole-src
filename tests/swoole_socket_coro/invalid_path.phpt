--TEST--
swoole_socket_coro: invalid path
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
use function Swoole\Coroutine\run;
run(function () {
    $socket = new Swoole\Coroutine\Socket(AF_INET, SOCK_STREAM, 0);
    $socket->sendFile("");
});
?>
--EXPECTF--
Fatal error: Uncaught ValueError: file to send is empty in %s:%d
Stack trace:
#0 %s
#1 [internal function]: {closure:%s:%d}()
#2 {main}
  thrown in %s on line %d
