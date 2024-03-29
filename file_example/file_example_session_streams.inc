<?php
/**
 * @file
 * Provides a demonstration session:// stream-wrapper.
 *
 * This example is nearly fully functional, but has no known
 * practical use. It's an example and demonstration only.
 */

/**
 * Example stream wrapper class to handle session:// streams.
 *
 * This is just an example, as it could have horrible results if much
 * information were placed in the $_SESSION variable. However, it does
 * demonstrate both the read and write implementation of a stream wrapper.
 *
 * A "stream" is an important Unix concept for the reading and writing of
 * files and other devices. Reading or writing a "stream" just means that you
 * open some device, file, internet site, or whatever, and you don't have to
 * know at all what it is. All the functions that deal with it are the same.
 * You can read/write more from/to the stream, seek a position in the stream,
 * or anything else without the code that does it even knowing what kind
 * of device it is talking to. This Unix idea is extended into PHP's
 * mindset.
 *
 * The idea of "stream wrapper" is that this can be extended indefinitely.
 * The classic example is HTTP: With PHP you can do a
 * file_get_contents("http://backdropcms.org/add-ons") as if it were a file,
 * because the scheme "http" is supported natively in PHP. So Backdrop adds
 * the public:// and private:// schemes, and contrib modules can add any
 * scheme they want to. This example adds the session:// scheme, which allows
 * reading and writing the $_SESSION['file_example'] key as if it were a file.
 *
 * Note that because this implementation uses simple PHP arrays ($_SESSION)
 * it is limited to string values, so binary files will not work correctly.
 * Only text files can be used.
 *
 * @ingroup file_example
 */
class FileExampleSessionStreamWrapper implements BackdropStreamWrapperInterface {
  /**
   * Stream context resource.
   *
   * @var Resource
   */
  public $context;


  /**
   * Instance URI (stream).
   *
   * These streams will be references as 'session://example_target'
   *
   * @var String
   */
  protected $uri;

  /**
   * The content of the stream.
   *
   * Since this trivial example just uses the $_SESSION variable, this is
   * simply a reference to the contents of the related part of
   * $_SESSION['file_example'].
   */
  protected $sessionContent;

  /**
   * Pointer to where we are in a directory read.
   */
  protected $directoryPointer;

  /**
   * List of keys in a given directory.
   */
  protected $directoryKeys;

  /**
   * The pointer to the next read or write within the session variable.
   */
  protected $streamPointer;

  /**
   * Constructor method.
   */
  public function __construct() {
    $_SESSION['file_example']['.isadir.txt'] = TRUE;
  }

  /**
   * Implements setUri().
   */
  public function setUri($uri) {
    $this->uri = $uri;
  }

  /**
   * Implements getUri().
   */
  public function getUri() {
    return $this->uri;
  }

  /**
   * Implements getTarget().
   *
   * The "target" is the portion of the URI to the right of the scheme.
   * So in session://example/test.txt, the target is 'example/test.txt'.
   */
  public function getTarget($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    list($scheme, $target) = explode('://', $uri, 2);

    // Remove erroneous leading or trailing, forward-slashes and backslashes.
    // In the session:// scheme, there is never a leading slash on the target.
    return trim($target, '\/');
  }

  /**
   * Implements getMimeType().
   */
  public static function getMimeType($uri, $mapping = NULL) {
    if (!isset($mapping)) {
      // The default file map, defined in file.mimetypes.inc is quite big.
      // We only load it when necessary.
      include_once BACKDROP_ROOT . '/core/includes/file.mimetypes.inc';
      $mapping = file_mimetype_mapping();
    }

    $extension = '';
    $file_parts = explode('.', basename($uri));

    // Remove the first part: a full filename should not match an extension.
    array_shift($file_parts);

    // Iterate over the file parts, trying to find a match.
    // For my.awesome.image.jpeg, we try:
    // - jpeg
    // - image.jpeg, and
    // - awesome.image.jpeg
    while ($additional_part = array_pop($file_parts)) {
      $extension = backdrop_strtolower($additional_part . ($extension ? '.' . $extension : ''));
      if (isset($mapping['extensions'][$extension])) {
        return $mapping['mimetypes'][$mapping['extensions'][$extension]];
      }
    }

    return 'application/octet-stream';
  }

  /**
   * Implements getDirectoryPath().
   *
   * In this case there is no directory string, so return an empty string.
   */
  public function getDirectoryPath() {
    return '';
  }

  /**
   * Overrides getExternalUrl().
   *
   * We have set up a helper function and menu entry to provide access to this
   * key via HTTP; normally it would be accessible some other way.
   */
  public function getExternalUrl() {
    $path = $this->getLocalPath();
    $url = url('examples/file_example/access_session/' . $path, array('absolute' => TRUE));

    return $url;
  }

  /**
   * We have no concept of chmod, so just return TRUE.
   */
  public function chmod($mode) {
    return TRUE;
  }

  /**
   * Implements realpath().
   */
  public function realpath() {
    return 'session://' . $this->getLocalPath();
  }

  /**
   * Returns the local path.
   *
   * Here we aren't doing anything but stashing the "file" in a key in the
   * $_SESSION variable, so there's not much to do but to create a "path"
   * which is really just a key in the $_SESSION variable. So something
   * like 'session://one/two/three.txt' becomes
   * $_SESSION['file_example']['one']['two']['three.txt'] and the actual path
   * is "one/two/three.txt".
   *
   * @param string $uri
   *   Optional URI, supplied when doing a move or rename.
   */
  protected function getLocalPath($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    $path  = str_replace('session://', '', $uri);
    $path = trim($path, '/');

    return $path;
  }

  /**
   * Opens a stream, as for fopen(), file_get_contents(), file_put_contents().
   *
   * @param string $uri
   *   A string containing the URI to the file to open.
   * @param string $mode
   *   The file mode ("r", "wb" etc.).
   * @param int $options
   *   A bit mask of STREAM_USE_PATH and STREAM_REPORT_ERRORS.
   * @param string &$opened_path
   *   A string containing the path actually opened.
   *
   * @return bool
   *   Returns TRUE if file was opened successfully. (Always returns TRUE).
   *
   * @see http://php.net/manual/en/streamwrapper.stream-open.php
   */
  public function stream_open($uri, $mode, $options, &$opened_path) {
    $this->uri = $uri;
    // We make $session_content a reference to the appropriate key in the
    // $_SESSION variable. So if the local path were
    // /example/test.txt it $session_content would now be a
    // reference to $_SESSION['file_example']['example']['test.txt'].
    $this->sessionContent = &$this->uri_to_session_key($uri);

    // Reset the stream pointer since this is an open.
    $this->streamPointer = 0;

    return TRUE;
  }

  /**
   * Return a reference to the correct $_SESSION key.
   *
   * @param string $uri
   *   The uri: session://something
   * @param bool $create
   *   If TRUE, create the key
   *
   * @return array|bool
   *   A reference to the array at the end of the key-path, or
   *   FALSE if the path doesn't map to a key-path (and $create is FALSE).
   */
  protected function &uri_to_session_key($uri, $create = TRUE) {
    // Since our uri_to_session_key() method returns a reference, we
    // have to set up a failure flag variable.
    $fail = FALSE;
    $path = $this->getLocalPath($uri);
    $path_components = explode('/', $path);
    // Set up a reference to the root session:// 'directory.'
    $var = &$_SESSION['file_example'];
    // Handle case of just session://.
    if (count($path_components) < 1) {
      return $var;
    }
    // Walk through the path components and create keys in $_SESSION,
    // unless we're told not to create them.
    foreach ($path_components as $component) {
      if ($create || isset($var[$component])) {
        $var = &$var[$component];
      }
      else {
        // This path doesn't exist as keys, either because the
        // key doesn't exist, or because we're told not to create it.
        return $fail;
      }
    }

    return $var;
  }

  /**
   * Support for flock().
   *
   * The $_SESSION variable has no locking capability, so return TRUE.
   *
   * @param int $operation
   *   One of the following:
   *   - LOCK_SH to acquire a shared lock (reader).
   *   - LOCK_EX to acquire an exclusive lock (writer).
   *   - LOCK_UN to release a lock (shared or exclusive).
   *   - LOCK_NB if you don't want flock() to block while locking (not
   *     supported on Windows).
   *
   * @return bool
   *   Always returns TRUE at the present time. (no support)
   *
   * @see http://php.net/manual/en/streamwrapper.stream-lock.php
   */
  public function stream_lock($operation) {
    return TRUE;
  }

  /**
   * Support for fread(), file_get_contents() etc.
   *
   * @param int $count
   *   Maximum number of bytes to be read.
   *
   * @return string
   *   The string that was read, or FALSE in case of an error.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-read.php
   */
  public function stream_read($count) {
    if (is_string($this->sessionContent)) {
      $remaining_chars = backdrop_strlen($this->sessionContent) - $this->streamPointer;
      $number_to_read = min($count, $remaining_chars);
      if ($remaining_chars > 0) {
        $buffer = backdrop_substr($this->sessionContent, $this->streamPointer, $number_to_read);
        $this->streamPointer += $number_to_read;
        return $buffer;
      }
    }

    return FALSE;
  }

  /**
   * Support for fwrite(), file_put_contents() etc.
   *
   * @param string $data
   *   The string to be written.
   *
   * @return int
   *   The number of bytes written (integer).
   *
   * @see http://php.net/manual/en/streamwrapper.stream-write.php
   */
  public function stream_write($data) {
    // Sanitize the data in a simple way since we're putting it into the
    // session variable.
    $data = check_plain($data);
    $this->sessionContent = substr_replace($this->sessionContent, $data, $this->streamPointer);
    $this->streamPointer += backdrop_strlen($data);

    return backdrop_strlen($data);
  }

  /**
   * Support for feof().
   *
   * @return bool
   *   TRUE if end-of-file has been reached.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-eof.php
   */
  public function stream_eof() {
    return FALSE;
  }

  /**
   * Support for fseek().
   *
   * @param int $offset
   *   The byte offset to got to.
   * @param int $whence
   *   SEEK_SET, SEEK_CUR, or SEEK_END.
   *
   * @return bool
   *   TRUE on success.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-seek.php
   */
  public function stream_seek($offset, $whence) {
    if (backdrop_strlen($this->sessionContent) >= $offset) {
      $this->streamPointer = $offset;
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Support for fflush().
   *
   * @return bool
   *   TRUE if data was successfully stored (or there was no data to store).
   *   This always returns TRUE, as this example provides and needs no
   *   flush support.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-flush.php
   */
  public function stream_flush() {
    return TRUE;
  }

  /**
   * Support for ftell().
   *
   * @return int
   *   The current offset in bytes from the beginning of file.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-tell.php
   */
  public function stream_tell() {
    return $this->streamPointer;
  }

  /**
   * Support for fstat().
   *
   * @return array
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-stat.php
   */
  public function stream_stat() {
    return array(
      'size' => backdrop_strlen($this->sessionContent),
    );
  }

  /**
   * Support for fclose().
   *
   * @return bool
   *   TRUE if stream was successfully closed.
   *
   * @see http://php.net/manual/en/streamwrapper.stream-close.php
   */
  public function stream_close() {
    $this->streamPointer = 0;
    // Unassign the reference.
    unset($this->sessionContent);

    return TRUE;
  }

  /**
   * Support for unlink().
   *
   * @param string $uri
   *   A string containing the uri to the resource to delete.
   *
   * @return bool
   *   TRUE if resource was successfully deleted.
   *
   * @see http://php.net/manual/en/streamwrapper.unlink.php
   */
  public function unlink($uri) {
    $path = $this->getLocalPath($uri);
    $path_components = preg_split('/\//', $path);
    $fail = FALSE;
    $unset = '$_SESSION[\'file_example\']';
    foreach ($path_components as $component) {
      $unset .= '[\'' . $component . '\']';
    }
    // TODO: Is there a better way to delete from an array?
    // backdrop_array_get_nested_value() doesn't work because it only returns
    // a reference; unsetting a reference only unsets the reference.
    eval("unset($unset);");

    return TRUE;
  }

  /**
   * Support for rename().
   *
   * @param string $from_uri
   *   The uri to the file to rename.
   * @param string $to_uri
   *   The new uri for file.
   *
   * @return bool
   *   TRUE if file was successfully renamed.
   *
   * @see http://php.net/manual/en/streamwrapper.rename.php
   */
  public function rename($from_uri, $to_uri) {
    $from_key = &$this->uri_to_session_key($from_uri);
    $to_key = &$this->uri_to_session_key($to_uri);
    if (is_dir($to_key) || is_file($to_key)) {
      return FALSE;
    }
    $to_key = $from_key;
    unset($from_key);

    return TRUE;
  }

  /**
   * Gets the name of the directory from a given path.
   *
   * @param string $uri
   *   A URI.
   *
   * @return string
   *   A string containing the directory name.
   *
   * @see backdrop_dirname()
   */
  public function dirname($uri = NULL) {
    list($scheme, $target) = explode('://', $uri, 2);
    $target  = $this->getTarget($uri);
    if (strpos($target, '/')) {
      $dirname = preg_replace('@/[^/]*$@', '', $target);
    }
    else {
      $dirname = '';
    }

    return $scheme . '://' . $dirname;
  }

  /**
   * Support for mkdir().
   *
   * @param string $uri
   *   A string containing the URI to the directory to create.
   * @param int $mode
   *   Permission flags - see mkdir().
   * @param int $options
   *   A bit mask of STREAM_REPORT_ERRORS and STREAM_MKDIR_RECURSIVE.
   *
   * @return bool
   *   TRUE if directory was successfully created.
   *
   * @see http://php.net/manual/en/streamwrapper.mkdir.php
   */
  public function mkdir($uri, $mode, $options) {
    // If this already exists, then we can't mkdir.
    if (is_dir($uri) || is_file($uri)) {
      return FALSE;
    }

    // Create the key in $_SESSION;
    $this->uri_to_session_key($uri, TRUE);

    // Place a magic file inside it to differentiate this from an empty file.
    $marker_uri = $uri . '/.isadir.txt';
    $this->uri_to_session_key($marker_uri, TRUE);

    return TRUE;
  }

  /**
   * Support for rmdir().
   *
   * @param string $uri
   *   A string containing the URI to the directory to delete.
   * @param int $options
   *   A bit mask of STREAM_REPORT_ERRORS.
   *
   * @return bool
   *   TRUE if directory was successfully removed.
   *
   * @see http://php.net/manual/en/streamwrapper.rmdir.php
   */
  public function rmdir($uri, $options) {
    $path = $this->getLocalPath($uri);
    $path_components = preg_split('/\//', $path);
    $fail = FALSE;
    $unset = '$_SESSION[\'file_example\']';
    foreach ($path_components as $component) {
      $unset .= '[\'' . $component . '\']';
    }
    // TODO: I really don't like this eval.
    debug($unset, 'array element to be unset');
    eval("unset($unset);");

    return TRUE;
  }

  /**
   * Support for stat().
   *
   * This important function goes back to the Unix way of doing things.
   * In this example almost the entire stat array is irrelevant, but the
   * mode is very important. It tells PHP whether we have a file or a
   * directory and what the permissions are. All that is packed up in a
   * bitmask. This is not normal PHP fodder.
   *
   * @param string $uri
   *   A string containing the URI to get information about.
   * @param int $flags
   *   A bit mask of STREAM_URL_STAT_LINK and STREAM_URL_STAT_QUIET.
   *
   * @return array|bool
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   *
   * @see http://php.net/manual/en/streamwrapper.url-stat.php
   */
  public function url_stat($uri, $flags) {
    // Get a reference to the $_SESSION key for this URI.
    $key = $this->uri_to_session_key($uri, FALSE);
    // Default to fail.
    $return = FALSE;
    $mode = 0;

    // We will call an array a directory and the root is always an array.
    if (is_array($key) && array_key_exists('.isadir.txt', $key)) {
      // S_IFDIR means it's a directory.
      $mode = 0040000;
    }
    elseif ($key !== FALSE) {
      // S_IFREG means it's a file.
      $mode = 0100000;
    }

    if ($mode) {
      $size = 0;
      if ($mode == 0100000) {
        $size = backdrop_strlen($key);
      }

      // There are no protections on this, so all writable.
      $mode |= 0777;
      $return = array(
        'dev' => 0,
        'ino' => 0,
        'mode' => $mode,
        'nlink' => 0,
        'uid' => 0,
        'gid' => 0,
        'rdev' => 0,
        'size' => $size,
        'atime' => 0,
        'mtime' => 0,
        'ctime' => 0,
        'blksize' => 0,
        'blocks' => 0,
      );
    }

    return $return;
  }

  /**
   * Support for opendir().
   *
   * @param string $uri
   *   A string containing the URI to the directory to open.
   * @param int $options
   *   Whether or not to enforce safe_mode (0x04).
   *
   * @return bool
   *   TRUE on success.
   *
   * @see http://php.net/manual/en/streamwrapper.dir-opendir.php
   */
  public function dir_opendir($uri, $options) {
    $var = &$this->uri_to_session_key($uri, FALSE);
    if ($var === FALSE || !array_key_exists('.isadir.txt', $var)) {
      return FALSE;
    }

    // We grab the list of key names, flip it so that .isadir.txt can easily
    // be removed, then flip it back so we can easily walk it as a list.
    $this->directoryKeys = array_flip(array_keys($var));
    unset($this->directoryKeys['.isadir.txt']);
    $this->directoryKeys = array_keys($this->directoryKeys);
    $this->directoryPointer = 0;

    return TRUE;
  }

  /**
   * Support for readdir().
   *
   * @return string|bool
   *   The next filename, or FALSE if there are no more files in the directory.
   *
   * @see http://php.net/manual/en/streamwrapper.dir-readdir.php
   */
  public function dir_readdir() {
    if ($this->directoryPointer < count($this->directoryKeys)) {
      $next = $this->directoryKeys[$this->directoryPointer];
      $this->directoryPointer++;
      return $next;
    }

    return FALSE;
  }

  /**
   * Support for rewinddir().
   *
   * @return bool
   *   TRUE on success.
   *
   * @see http://php.net/manual/en/streamwrapper.dir-rewinddir.php
   */
  public function dir_rewinddir() {
    $this->directoryPointer = 0;

    return TRUE;
  }

  /**
   * Support for closedir().
   *
   * @return bool
   *   TRUE on success.
   *
   * @see http://php.net/manual/en/streamwrapper.dir-closedir.php
   */
  public function dir_closedir() {
    $this->directoryPointer = 0;
    unset($this->directoryKeys);

    return TRUE;
  }
}
