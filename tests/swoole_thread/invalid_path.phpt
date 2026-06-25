--TEST--
swoole_thread: invalid path
--SKIPIF--
<?php
require __DIR__ . '/../include/skipif.inc';
skip_if_nts();
?>
--FILE--
<?php
require __DIR__ . '/../include/bootstrap.php';

use Swoole\Thread;
$thread = new Thread('', '');
?>
--EXPECT--
exec file name is empty
