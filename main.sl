// a = 10
// b = 20
$a = 10;
$b = 20;

// simple_maths = 16
$simple_maths = ($a + $b) / 2 + 1; 

// tup_a = 10
// tup_b = 20
($tup_a, $tup_b) = ($a, $b);


// sum = 55
$sum = 0, $array = [1,2,3,4,5,6,7,8,9,10];
for($i = 0; $i < 10; $i++) {
	$sum += $array[$i];
}
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

// factorial = 120
function factorial($n = 5) {
	if($n == 1) {
		return 1;
	}
	return $n * factorial($n - 1);
}
$factorial = factorial();

// i = 0
$i = 1;
while($i != 0) {
	$i = 0;
}