--TEST--
swoole_server/task: finish timeout
--SKIPIF--
<?php require __DIR__ . '/../../include/skipif.inc';
skip_if_darwin_todo();
?>
--FILE--
<?php
require __DIR__ . '/../../include/bootstrap.php';

use Swoole\Client;
use Swoole\Server;
use SwooleTest\ProcessManager;

define('TMP_LOG_FILE', __DIR__ . '/log_file');
ftruncate(fopen(TMP_LOG_FILE, 'w+'), 0);

$pm = new ProcessManager();
$pm->parentFunc = function ($pid) use ($pm) {
    $cli = new Client(SWOOLE_SOCK_TCP, SWOOLE_SOCK_SYNC);
    $cli->connect('127.0.0.1', $pm->getFreePort(), 10) or exit('ERROR');
    $cli->send('task-01') or exit('ERROR');
    Assert::same($cli->recv(), 'hello world');
    $cli->close();
    usleep(1000000);
    echo file_get_contents(TMP_LOG_FILE);
    $pm->kill();
};

$pm->childFunc = function () use ($pm) {
    swoole_ignore_error(SWOOLE_ERROR_SERVER_NO_IDLE_WORKER);
    $server = new Server('127.0.0.1', $pm->getFreePort(), SWOOLE_PROCESS);
    $server->set([
        'log_file' => TMP_LOG_FILE,
        'log_level' => SWOOLE_LOG_NOTICE,
        'task_worker_num' => 1,
        'socket_send_timeout' => 1.0,
        'socket_buffer_size' => 128 * 1024,
        'worker_num' => 1,
        'enable_coroutine' => false,
    ]);
    $server->on('workerStart', function () use ($pm) {
        $pm->wakeup();
    });
    $server->on('receive', function (Server $server, $fd, $tid, $data) {
        $server->task($fd);
        usleep(1100000);
    });
    $server->on('task', function ($server, $task_id, $worker_id, string $fd) {
        $n = 200;
        $size = IS_MAC_OS ? 2000 : 8000;
        while ($n--) {
            if (!$server->finish(str_repeat('A', $size))) {
                break;
            }
        }
        $server->send($fd, 'hello world');
    });
    $server->on('finish', function ($server, $task_id, $data) {
        
    });
    $server->on('close', function () {});
    $server->start();
};
$pm->childFirst();
$pm->run();
unlink(TMP_LOG_FILE);
?>
--EXPECTF--
[%s]	WARNING	Server::finish() (ERRNO %d): send result to worker timed out
