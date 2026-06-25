--TEST--
swoole_http_server: invalid path
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';

use Swoole\Http\Server;
use Swoole\Http\Request;
use Swoole\Http\Response;

$pm = new ProcessManager;
$pm->parentFunc = function ($pid) use ($pm) {
    file_get_contents("http://127.0.0.1:{$pm->getFreePort()}");;
    $pm->kill();
};
$pm->childFunc = function () use ($pm) {
    $http = new Server('127.0.0.1', $pm->getFreePort(), SERVER_MODE_RANDOM);
    $http->set([
        'worker_num' => 1,
        'log_file' => '/dev/null'
    ]);
    $http->on('workerStart', function () use ($pm) {
        $pm->wakeup();
    });
    $http->on('request', function (Request $request, Response $response) {
        $response->header('Content-Type', 'application/octet-stream');
        $response->header('Content-Disposition', 'attachment; filename=recvfile.txt');
        $response->sendfile('');
    });
    $http->start();
};
$pm->childFirst();
$pm->run();
?>
--EXPECTF--
Warning: Swoole\Http\Response::sendfile(): file name is empty in %s on line %d

Warning: file_get_contents(%s): Failed to open stream: HTTP request failed! HTTP/1.1 500 Internal Server Error
 in %s on line %d
