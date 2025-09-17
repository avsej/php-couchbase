<pre>
<?php

$pid = getmypid();
printf("pid=%d\n", $pid);

printf("include_path=%s\n", ini_get("include_path"));
printf("error_log=%s\n", ini_get("error_log"));

printf("couchbase.log_level=%s\n", ini_get("couchbase.log_level"));
printf("couchbase.log_path=%s\n", ini_get("couchbase.log_path"));
printf("couchbase.log_php_log_err=%s\n", ini_get("couchbase.log_php_log_err"));
printf("couchbase.log_stderr=%s\n", ini_get("couchbase.log_stderr"));
printf("couchbase.max_persistent=%s\n", ini_get("couchbase.max_persistent"));
printf("couchbase.persistent_timeout=%s\n", ini_get("couchbase.persistent_timeout"));

$iterations = (int)getParameter("iterations", "1");
$delay = (int)getParameter("delay", "0");
printf("perform %d upserts and sleep for %d seconds after each iteration\n", $iterations, $delay);
ob_flush();
flush();

function getParameter(string $paramName, string $default): string {
    $param = $_GET[$paramName] ?? null;
    return $param ? urldecode($param) : $default;
}

function getRandomParameter(string $paramName, string $default): string {
    $params = $_GET[$paramName] ?? null;
    if ($params == null) {
        return $default;
    }
    if (!is_array($params)) {
        $params = [$params];
    }
    return $params[array_rand($params)];
}

function addParameter($connection_string, $name, $value) {
    $separator = '?';

    if (strpos($connection_string, '?') !== false) {
        $last_char = substr($connection_string, -1);
        if ($last_char !== '?' && $last_char !== '&') {
            $separator = '&';
        } else {
            $separator = '';
        }
    }

    return $connection_string . $separator . urlencode($name) . '=' . urlencode($value);
}

$username = getParameter("username", "Administrator");
$password = getParameter("password", "password");
$connection_string = getParameter("connection_string", "couchbase://localhost");
$connection_string = addParameter($connection_string, "client_string", sprintf("pid_%d", $pid));
$bucket_name = getRandomParameter("bucket", "default");
$scope_name = getRandomParameter("scope", "_default");
$collection_name = getParameter("collection", "_default");
$key = getParameter("key", "foo");
printf("username: %s\n", $username);
printf("password: %s\n", $password);
printf("connection_string: %s\n", $connection_string);
printf("bucket_name: %s\n", $bucket_name);
printf("scope_name: %s\n", $scope_name);
printf("collection_name: %s\n", $collection_name);
printf("key: %s\n", $key);
ob_flush();
flush();

use Couchbase\ClusterOptions;
use Couchbase\Cluster;

$options = new ClusterOptions();
$options->credentials($username, $password);

$cluster = new Cluster($connection_string, $options);
$collection = $cluster->bucket($bucket_name)
                      ->scope($scope_name)
                      ->collection($collection_name);

for ($i = 0; $i < $iterations; $i++) {
  $res = $collection->upsert($key, ["value" => 42]);
  var_dump($res);
  ob_flush();
  flush();
  if ($delay > 0) {
    sleep($delay);
  }
}
?>
</pre>

