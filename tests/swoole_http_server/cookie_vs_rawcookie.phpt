--TEST--
swoole_http_server: cookie vs rawcookie
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
$pm = new ProcessManager;
$pm->parentFunc = function () use ($pm) {
    go(function () use ($pm) {
        $cli = new Swoole\Coroutine\Http\Client('127.0.0.1', $pm->getFreePort());
        $cookie = '123_,; abc';
        $cookie_encoded = urlencode($cookie);
        Assert::assert($cli->get('/?cookie=' . $cookie_encoded));
        Assert::same($cli->statusCode, 200);
        Assert::eq($cli->set_cookie_headers, [
            'cookie=' . $cookie_encoded,
            'rawcookie=' . $cookie_encoded,
        ]);
    });
    for ($i = MAX_CONCURRENCY_LOW; $i--;) {
        go(function () use ($pm) {
            $cli = new Swoole\Coroutine\Http\Client('127.0.0.1', $pm->getFreePort());
            $random = get_safe_random();
            Assert::assert($cli->get('/?cookie=' . $random));
            Assert::same($cli->statusCode, 200);
            Assert::assert($cli->set_cookie_headers ===
                [
                    'cookie=' . urlencode($random),
                    'rawcookie=' . $random
                ]
            );
        });
    }
    Swoole\Event::wait();
    echo "SUCCESS\n";
    $pm->kill();
};
$pm->childFunc = function () use ($pm) {
    $http = new Swoole\Http\Server('0.0.0.0', $pm->getFreePort(), SWOOLE_BASE);
    $http->set(['worker_num' => 1, 'log_file' => '/dev/null']);
    $http->on('request', function (Swoole\Http\Request $request, Swoole\Http\Response $response) {
        $cookie = $request->get['cookie'];
        $response->cookie('cookie', $cookie);
        $response->rawcookie('rawcookie', urlencode($cookie));
        $response->end();
    });
    $http->start();
};
$pm->childFirst();
$pm->run();
?>
--EXPECTF--
SUCCESS
