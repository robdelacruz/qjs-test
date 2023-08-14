import * as os from "os";
import * as std from "std";
import * as testlib from "testlib";

//print(Object.getPrototypeOf(testlib));

var w = testlib.new_win();
w.set_title("New Window Title");
w.set_size(640, 480);

print(`w.title: '${w.title}'`);
print(`w.width: ${w.width}`);
print(`w.height: ${w.height}`);

