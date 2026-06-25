--TEST--
swoole_server: invalid path
--SKIPIF--
<?php
require __DIR__ . '/../include/skipif.inc';
?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';

use Swoole\Server;
use Swoole\Constant;

$pm = new SwooleTest\ProcessManager;

$pm->parentFunc = function ($pid) use ($pm) {
    go(function () use ($pm) {
        $client = new Swoole\Coroutine\Client(SWOOLE_SOCK_TCP);
        $client->set(['socket_buffer_size' => 128*1024]);
        if (!$client->connect('127.0.0.1', $pm->getFreePort())) {
            exit("connect failed\n");
        }
        $client->send("close");
        $client->close();
    });
    Swoole\Event::wait();
    $pm->kill();
};

$pm->childFunc = function () use ($pm) {
    $serv = new Server('127.0.0.1', $pm->getFreePort(), SERVER_MODE_RANDOM);
    $serv->set([
        'worker_num' => 1,
        'log_file' => TEST_LOG_FILE
    ]);
    $serv->on("workerStart", function ($serv) use ($pm) {
        $pm->wakeup();
    });
    $serv->on('receive', function (Server $serv, $fd, $reactor_id, $data) {
        $serv->sendfile($fd, "");
        $serv->close($fd);
    });

    $serv->start();
};

$pm->childFirst();
$pm->run();
?>
--EXPECTF--
Fatal error: Uncaught ValueError: file to send is empty in %s:%d
Stack trace:
#0 %s
#1 [internal function]: %s
#2 {main}
  thrown in %s on line %d
