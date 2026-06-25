--TEST--
swoole_http_client_coro: invalid path
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc';
skip_if_offline();
?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
use Swoole\Coroutine\Http\Client;
use function Swoole\Coroutine\run;

run(function() {
    $cli = new Client('127.0.0.1', get_one_free_port(), true);
    $cli->addFile('', 0);
});
?>
--EXPECTF--
Fatal error: Uncaught ValueError: path cannot be empty in %s:%d
Stack trace:
#0 %s
#1 [internal function]: %s
#2 {main}
  thrown in %s on line %d
