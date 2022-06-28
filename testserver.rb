#!/usr/bin/env ruby1.9.1

$:.unshift(File.join(File.dirname(__FILE__), "lib", "json_pure-1.5.3", "lib"))
$:.unshift(File.join(File.dirname(__FILE__), "lib"))

require 'webrick'

require 'testsuite_runner.rb'
require 'servlets.rb'

STATIC_PATH           = File.absolute_path(File.join(File.dirname(__FILE__), "lib", "static"))
TEST_CASES_PATH       = File.absolute_path(File.join(File.dirname(__FILE__), "test_cases"))

DEFAULT_PORT = 18000

def start_webrick(config = {})
  config[:Port] ||= DEFAULT_PORT

  server = WEBrick::HTTPServer.new(config)

  yield server if block_given?

  ['INT', 'TERM'].each do |signal|
    trap(signal) { server.shutdown }
  end
  server.start

end


if $0 == __FILE__ then
  # TODO parse arguments (such as port, emulate assemble path)

  emulate_assemble_path = nil
  port = DEFAULT_PORT

  case ARGV.length
  when 1
    emulate_assemble_path = ARGV[0]
  when 2
    emulate_assemble_path = ARGV[0]
    port = ARGV[1]
  else
    puts <<-EOH
Usage: testserver.rb emulate_assemble_path [port]
  where
    <emulate_assemble_path> is the path to the directory containing the
                            emulate and assemble binaries.

    <port> is the port to start the server on.  Defaults to #{DEFAULT_PORT}.

    One the server is running, point a (modern) webrowser at
    http://localhost:<port> to bring up the test-suite gui.
    EOH
    exit 0
  end

  system_mime_table = WEBrick::HTTPUtils::load_mime_types('/etc/mime.types')
  my_mime_table = system_mime_table.update({ "js" => "application/javascript" })

  puts "*******************************************************"
  puts "*** Starting server on port #{port}                   ***"
  puts "*** Visit http://localhost:#{port} to view testsuite. ***"
  puts "*******************************************************"

  start_webrick(:DocumentRoot => STATIC_PATH,
                :MimeTypes => my_mime_table,
               :Port => port) do |server|

    # add servlets

    test_suite_runner = TestSuiteRunner.new(TEST_CASES_PATH, emulate_assemble_path)

    server.mount('/api/status', ServletStatus, test_suite_runner)
    server.mount('/api/run', ServletRun, test_suite_runner)
    server.mount('/api/run_one', ServletRunOne, test_suite_runner)
    server.mount('/files/source', ServletTestSourceFile, TEST_CASES_PATH)
    server.mount('/files/binary', ServletTestBinaryFile, TEST_CASES_PATH)

  end

end
