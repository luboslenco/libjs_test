
// Reference: https://github.com/SerenityOS/serenity/blob/master/Meta/Lagom/Wasm/js_repl.cpp

#include <LibCore/ConfigFile.h>
#include <LibJS/Bytecode/Interpreter.h>
#include <LibJS/Print.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

RefPtr<JS::VM> g_vm;

class MyStream final : public Stream {
	virtual ErrorOr<Bytes> read_some(Bytes) override { return Error::from_string_view("Not readable"sv); }
	virtual ErrorOr<size_t> write_some(ReadonlyBytes bytes) override {
		char buf[1024];
		strncpy(buf, bit_cast<char const*>(bytes.data()), bytes.size());
		buf[bytes.size()] = 0;
		printf("%s\n", buf);
		return bytes.size();
	}
	virtual bool is_eof() const override { return true; }
	virtual bool is_open() const override { return true; }
	virtual void close() override { }
};

ErrorOr<void> print(JS::Value value) {
	MyStream stream;
	JS::PrintContext print_context {
		.vm = *g_vm,
		.stream = stream,
		.strip_ansi = true,
	};
	return JS::print(value, print_context);
}

class MyObject final : public JS::GlobalObject {
	JS_OBJECT(MyObject, JS::GlobalObject);
public:
	MyObject(JS::Realm& realm) : GlobalObject(realm) {}
	virtual void initialize(JS::Realm&) override;
	JS_DECLARE_NATIVE_FUNCTION(add);
};

static void parse_and_run(JS::Realm& realm, StringView source) {
	auto script_or_error = JS::Script::parse(source, realm, "TEST"sv);
	if (script_or_error.is_error()) {
		printf("Error\n");
	}
	else {
		auto& interpreter = g_vm->bytecode_interpreter();
		JS::ThrowCompletionOr<JS::Value> result = interpreter.run(*script_or_error.value());
		(void)print(result.value());
	}
}

void MyObject::initialize(JS::Realm& realm) {
    Base::initialize(realm);
    u8 attr = JS::Attribute::Configurable | JS::Attribute::Writable | JS::Attribute::Enumerable;
    define_native_function(realm, "add", add, 2, attr);
}

JS_DEFINE_NATIVE_FUNCTION(MyObject::add) {
    double i = vm.argument(0).as_double();
    double j = vm.argument(1).as_double();
    return JS::Value(i + j);
}

int main() {
	g_vm = MUST(JS::VM::create());
	OwnPtr<JS::ExecutionContext> execution_context = JS::create_simple_execution_context<MyObject>(*g_vm);
	const char *source = "let ar = new Uint8Array(2048 * 2048); for (let i = 0; i < ar.length; ++i) { ar[i] = i * 2; } add(ar[1], ar[2]);";
	parse_and_run(*execution_context->realm, { source, strlen(source) });
	return 0;
}
