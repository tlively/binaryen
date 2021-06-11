;; RUN: not wasm-opt %s -all --nominal -S -o - 2>&1 | filecheck %s

;; CHECK: [wasm-validator error in function ret-foo] function body type must match

(module
  (type $foo (struct i32))
  (type $bar (struct i32))

  (global $foo.rtt (rtt $foo) (rtt.canon $foo))
  (global $bar.rtt (rtt $bar) (rtt.canon $bar))

  (func $ret-foo (result (ref null $foo))
    (struct.new_with_rtt $bar
      (i32.const 42)
      (global.get $bar.rtt)
    )
  )
)