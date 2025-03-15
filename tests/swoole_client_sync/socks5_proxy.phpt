--TEST--
swoole_client_sync: http client with http_proxy
--SKIPIF--
<?php
require __DIR__ . '/../include/skipif.inc';
skip_if_no_http_proxy();
skip_if_offline();
?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
require TESTS_API_PATH . '/swoole_client/http_get.php';

$cli = new Swoole\Client(SWOOLE_TCP);
$cli->set([
    'timeout' => 30,
    'socks5_host' => '127.0.0.1',
    'socks5_port' => 10801
]);
client_http_v10_get($cli)
?>
--EXPECT--
