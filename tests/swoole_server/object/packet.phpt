--TEST--
swoole_server/object: packet object
--SKIPIF--
<?php require __DIR__ . '/../../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../../include/bootstrap.php';

use Swoole\Coroutine\Client;
use Swoole\Server;
use Swoole\Server\Packet;
use SwooleTest\ProcessManager;

use function Swoole\Coroutine\run;

$pm = new ProcessManager();

$pm->parentFunc = function ($pid) use ($pm) {
    run(function () use ($pm) {
        $client = new Client(SWOOLE_SOCK_UDP);
        if (!$client->connect('127.0.0.1', $pm->getFreePort())) {
            echo 'Over flow. errno=' . $client->errCode;
            exit("\n");
        }

        $data = base64_encode(random_bytes(rand(1024, DGRAM_MAX_SIZE))) . "\r\n\r\n";
        $client->send($data);
        $recv_data = $client->recv();
        Assert::eq($recv_data, $data);
    });
    $pm->kill();
};

$pm->childFunc = function () use ($pm) {
    $serv = new Server('127.0.0.1', $pm->getFreePort(), SWOOLE_BASE, SWOOLE_SOCK_UDP);
    $serv->set(
        [
            'worker_num' => 1,
            'event_object' => true,
            'log_file' => '/dev/null',
        ]
    );
    $serv->on(
        'WorkerStart',
        function (Server $serv) use ($pm) {
            $pm->wakeup();
        }
    );
    $serv->on(
        'packet',
        function (Server $serv, Packet $object) {
            $serv->sendto($object->address, $object->port, $object->data, $object->server_socket);
        }
    );
    $serv->start();
};

$pm->childFirst();
$pm->run();
?>
--EXPECT--
