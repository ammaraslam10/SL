# SL
A Simple Language (PHP-Like Syntax)

Usage: SL.exe &lt;filename&gt;

A 64-bit windows executable is provided (SL.exe) alongside some demo SL code (main.sl).

Example:

**Declare Variables:**
```php
// a = 10
// b = 20
$a = 10;
$b = 20;
```

**Maths:**
```php
// simple_maths = 16
$simple_maths = ($a + $b) / 2 + 1; 
```

**Loops:**
```php
// i = 0
$i = 1;
while($i != 0) {
	$i = 0;
}
```

**Arrays:**
```php
// sum = 55
$sum = 0, $array = [1,2,3,4,5,6,7,8,9,10];
for($i = 0; $i < 10; $i++) {
	$sum += $array[$i];
}
```

**Array Manipulation:**
```php
$array["hello"] = "world";
$array[0] = [
	"SubArray", 
	100, 
	$i + 1, 
	[
		1, 
		2, 
		"SubSubArray"
	]
];
```

**Tuples & Tuple destructuring:**
```php
// tup_a = 10
// tup_b = 20
($tup_a, $tup_b) = ($a, $b);
```

**Functions:**
```php
// factorial = 120
function factorial($n = 5) {
	if($n == 1) {
		return 1;
	}
	return $n * factorial($n - 1);
}
$factorial = factorial();
```

Currently there are no built-in functions & no object support. Variables that are declared are simply presented on the screen after execution.
