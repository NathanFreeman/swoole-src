--TEST--
swoole_http_server: duplicate header
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';

$pm = new ProcessManager;

$pm->parentFunc = function () use ($pm) {
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, "http://127.0.0.1:{$pm->getFreePort()}/");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_HTTPHEADER, [
        'X-Test-Header1: value1',
        'X-Test-Header2: value2',
        'X-Test-Header2: value3',
        'X-Test-Header3: value4',
        'X-Test-Header3: value5',
        'X-Test-Header3: value6',
    ]);
    curl_setopt($ch, CURLOPT_HEADER, true);
    echo curl_exec($ch);
    curl_close($ch);
    $pm->kill();
};

$pm->childFunc = function () use ($pm) {
    $http = new Swoole\Http\Server('127.0.0.1', $pm->getFreePort(), SWOOLE_BASE);
    $http->set([
        'worker_num' => 1,
        'enable_coroutine' => false,
        'log_file' => '/dev/null'
    ]);
    $http->on('workerStart', function () use ($pm) {
        $pm->wakeup();
    });
    $http->on('request', function (Swoole\Http\Request $request, Swoole\Http\Response $response) {
        $msg = "hello world";
        Assert::eq($request->header['x-test-header1'], 'value1');
        Assert::eq($request->header['x-test-header2'], ['value2', 'value3']);
        Assert::eq($request->header['x-test-header3'], ['value4', 'value5', 'value6']);
        $response->header("content-length", strlen($msg) . " ");
        $response->header("Test-Value", [
            "a\r\n",
            "b1234 ",
            "d5678",
            "e  \n ",
            null,
            5678,
            3.1415926,
        ]);
        $response->end($msg);
    });
    $http->start();
};

$pm->childFirst();
$pm->run();
?>
--EXPECTF--
HTTP/1.1 200 OK
Content-Length: 11
Test-Value: a
Test-Value: b1234
Test-Value: d5678
Test-Value: e
Test-Value: 5678
Test-Value: 3.1415926
Server: swoole-http-server
Date: %s
Connection: keep-alive
Content-Type: text/html

hello world
