--TEST--
NameResolver: re-entering a Context while it is being reset is rejected, not freed twice
--SKIPIF--
<?php if (!extension_loaded('swoole')) die('skip swoole required'); ?>
--ENV--
USE_ZEND_ALLOC=0
--FILE--
<?php
function same($actual, $expected): void {
    if ($actual !== $expected) {
        throw new RuntimeException(var_export([$actual, $expected], true));
    }
}
/* Destroying the Cluster retained by the Context re-enters that same Context. */
class ReentrantCluster extends Swoole\NameResolver\Cluster {
    public static int $armed = 0;
    public static int $reentered = 0;
    public static int $rejected_construct = 0;
    public static int $rejected_lookup = 0;
    private array $queue;
    public function __construct() {
        $this->queue = [['host' => '127.0.0.2', 'port' => 8080], ['host' => '127.0.0.3', 'port' => 8081]];
    }
    public function pop(): array|false { return array_shift($this->queue) ?? false; }
    public function count(): int { return count($this->queue); }
    public function __destruct() {
        if (self::$armed === 0) {
            return;
        }
        self::$armed = 0;
        self::$reentered++;
        global $ctx;
        // The Context is mid-reset: both entry points must be rejected before touching its state.
        try {
            $ctx->__construct(AF_INET);
        } catch (Error $e) {
            self::$rejected_construct = $e->getMessage() === 'NameResolver Context is already in use' ? 1 : 2;
        }
        try {
            swoole_name_resolver_lookup('other', $ctx);
        } catch (Error $e) {
            self::$rejected_lookup = $e->getMessage() === 'NameResolver Context is already in use' ? 1 : 2;
        }
    }
}
$resolver = new class extends Swoole\NameResolver {
    public function __construct() {}
    public function join(string $name, string $ip, int $port, array $options = []): bool { return true; }
    public function leave(string $name, string $ip, int $port): bool { return true; }
    public function getCluster(string $name): ?Swoole\NameResolver\Cluster { return null; }
    public function lookup(string $name) { return new ReentrantCluster(); }
};
same(swoole_name_resolver_add($resolver), true);
try {
    $ctx = new Swoole\NameResolver\Context(AF_INET);
    // The Context retains a clone of the returned Cluster, a second node keeps it a Cluster.
    same(swoole_name_resolver_lookup('svc', $ctx), '127.0.0.2');
    same(ReentrantCluster::$reentered, 0);
    ReentrantCluster::$armed = 1;
    // Resetting the Context destroys the retained clone; its destructor re-enters the Context.
    $ctx->__construct(AF_INET);
    same(ReentrantCluster::$reentered, 1);
    same(ReentrantCluster::$rejected_construct, 1);
    same(ReentrantCluster::$rejected_lookup, 1);
    // The retained data must have been freed exactly once: the Context is still usable.
    same(swoole_name_resolver_lookup('svc', $ctx), '127.0.0.2');
} finally {
    same(swoole_name_resolver_remove($resolver), true);
}
echo "DONE\n";
?>
--EXPECT--
DONE
