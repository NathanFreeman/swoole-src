--TEST--
swoole_client: invalid path
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
use Swoole\Client;

$client = new Client(SWOOLE_SOCK_TCP, SWOOLE_SOCK_SYNC);
$client->sendfile('', 0);
?>
--EXPECTF--
Warning: Swoole\Client::sendfile(): file to send is empty in %s on line %d
