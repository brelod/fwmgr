import json
from unittest import TestCase, main
from unittest.mock import patch, Mock
from server import Response, Runner, Process, Connection
from server import ValidationError, ExecutionError
from server import OK, INVALID_METHOD, INVALID_HOST, INVALID_JSON, EXECUTION_ERROR


class TestReponse(TestCase):
    def test_repr(self):
        self.assertEqual(repr(Response(0, "my-msg")), "Response(code=0, msg=my-msg)")

    def test_init_code(self):
        self.assertEqual(Response(0, "my-msg").code, 0)

    def test_init_msg(self):
        self.assertEqual(Response(0, "my-msg").msg, "my-msg")

    def test_dump(self):
        resp = Response(0, "my-msg")
        self.assertEqual(resp.dump(), b'{"code": 0, "msg": "my-msg"}')


class MockRunner(Runner):
    cmd = None

    def __init__(self, method='append', host='1.2.3.4', process=None):
        data = bytes(json.dumps({
            'method': method,
            'host': host,
        }), encoding='utf8')

        super().__init__(data)

        if process is None:
            process = Process(return_code=0, stdout=b'stdout', stderr=b'stderr')

        self.process = process

    def _run(self, cmd):
        self.cmd = cmd
        return self.process


class TestRunner(TestCase):
    def test_methods(self):
        self.assertEqual(set(Runner.METHODS) - set(dir(Runner)), set())

    def test_repr(self):
        self.assertEqual(repr(Runner('my-data')), 'Runner(my-data)')

    def test_init_data(self):
        self.assertEqual(Runner('my-data').data, 'my-data')

    @patch('server.logger') 
    @patch('server.Popen') 
    def test__run_non_zero_return_code(self, popen, logger):
        process = popen.return_value
        process.poll.return_value = 1    # return code from process.poll() function
        process.communicate.return_value = ("stdout", "stderr")
        runner = Runner('my-data')
        runner._run('my-cmd')
        self.assertEqual(logger.warning.call_args.args, ('Return-code: %s; stdout: %s; stderr: %s', 1, 'stdout', 'stderr'))

    @patch('server.logger') 
    @patch('server.Popen') 
    def test__run_return_value(self, popen, logger):
        process = popen.return_value
        process.poll.return_value = 0    # return code from process.poll() function
        process.communicate.return_value = ("stdout", "stderr")
        runner = Runner('my-data')
        result = runner._run('my-cmd')
        self.assertIs(result.__class__, Process)
        self.assertEqual(result.return_code, 0)
        self.assertEqual(result.stdout, 'stdout')
        self.assertEqual(result.stderr, 'stderr')

    def test_execute_without_error(self):
        runner = MockRunner()
        response = runner.execute()
        self.assertEqual(response.code, 0) 
        self.assertRegex(response.msg, 'has been successfully appended')

    def test_execute_with_error(self):
        runner = MockRunner(method='not-existing')
        response = runner.execute()
        self.assertNotEqual(response.code, 0) 
        self.assertRegex(response.msg, 'Method "not-existing" does not exist')

    def test_parse_without_error(self):
        runner = MockRunner(method='append', host='1.2.3.4')
        method, addr = runner.parse()
        self.assertEqual(method, 'append')
        self.assertEqual(addr, '1.2.3.4')

    def test_parse_without_method(self):
        runner = MockRunner()
        runner.data = bytes(json.dumps({'host': '1.2.3.4'}), encoding='utf8')
        method, addr = runner.parse()
        self.assertIs(method, None)
        self.assertEqual(addr, '1.2.3.4')

    def test_parse_without_host(self):
        runner = MockRunner()
        runner.data = bytes(json.dumps({'method': 'append'}), encoding='utf8')
        method, addr = runner.parse()
        self.assertEqual(method, 'append')
        self.assertIs(addr, None)

    def test_parse_with_json_error(self):
        runner = MockRunner()
        runner.data = b""
        self.assertRaises(ValidationError, runner.parse)

    def test_validate_method_missing(self):
        runner = MockRunner()
        self.assertRaises(ValidationError, runner.validate, method=None, addr='1.2.3.4')

    def test_validate_method_invalid(self):
        runner = MockRunner()
        self.assertRaises(ValidationError, runner.validate, method='invalid', addr='1.2.3.4')

    def test_validate_address_missing(self):
        runner = MockRunner()
        self.assertRaises(ValidationError, runner.validate, method='append', addr=None)

    def test_validate_address_invalid(self):
        runner = MockRunner()
        self.assertRaises(ValidationError, runner.validate, method='append', addr='invalid')

    def test_append_success(self):
        runner = MockRunner()
        response = runner.append('1.2.3.4')
        self.assertEqual(runner.cmd, "iptables -A FORWARD -s 1.2.3.4 -j ACCEPT")
        self.assertEqual(response.code, OK)
        self.assertEqual(response.msg, "Host 1.2.3.4 has been successfully appended")

    def test_append_error(self):
        runner = MockRunner(process=Process(return_code=1, stdout='', stderr=''))
        self.assertRaises(ExecutionError, runner.append, '1.2.3.4')

    def test_remove_success(self):
        runner = MockRunner()
        response = runner.remove('1.2.3.4')
        self.assertEqual(runner.cmd, "iptables -D FORWARD -s 1.2.3.4 -j ACCEPT")
        self.assertEqual(response.code, OK)
        self.assertEqual(response.msg, "Host 1.2.3.4 has been successfully removed")

    def test_remove_error_general(self):
        runner = MockRunner(process=Process(return_code=1, stdout=b'', stderr=b''))
        self.assertRaises(ExecutionError, runner.remove, '1.2.3.4')

    def test_remove_error_rule_not_exist(self):
        runner = MockRunner(process=Process(return_code=1, stdout=b'', stderr=b'does a matching rule exist'))
        response = runner.remove('1.2.3.4')
        self.assertEqual(response.code, EXECUTION_ERROR)
        self.assertEqual(response.msg, "No matching rule presents")


class TestConnection(TestCase):
    def test_init_socket(self):
        socket = 'socket'
        self.assertIs(Connection(socket, None).socket, socket)

    def test_init_addr(self):
        addr = 'addr'
        self.assertIs(Connection(None, addr).addr, addr)

    @patch('server.Runner.execute')
    def test_run(self, execute):
        socket = Mock()
        socket.recv.return_value = bytes(json.dumps({'method': 'append', 'host': '1.2.3.4'}), encoding='utf8')
        execute.return_value = Response(code=0, msg="my-response")
        conn = Connection(socket, ('1.2.3.4', 5555))
        conn.run()
        self.assertEqual(socket.send.call_args.args, (b'{"code": 0, "msg": "my-response"}',))
        self.assertEqual(socket.close.call_count, 1)



if __name__ == "__main__":
    main()





















