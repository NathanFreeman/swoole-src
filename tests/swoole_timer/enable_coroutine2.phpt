--TEST--
swoole_timer: enable_coroutine setting
--SKIPIF--
<?php require __DIR__ . '/../include/skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';
swoole_async_set([
    'enable_coroutine' => false
]);
Swoole\Timer::after(1, function () {
    $uid = Co::getuid();
    echo "#{$uid}\n";
});
Swoole\Event::wait();

swoole_async_set([
    'enable_coroutine' => true
]);
Swoole\Timer::after(1, function () {
    $uid = Co::getuid();
    echo "#{$uid}\n";
});
Swoole\Event::wait();
?>
--EXPECT--
#-1
#1
