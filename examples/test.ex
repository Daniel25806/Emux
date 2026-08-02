[Vars]
V0 : u8
V1 : u8

[Alias]

func _Start(i8 i) -> u8 {
	V1 = 10
	return 0
}

[Main: Alias]
 
func _Start(i8 i) -> u8 {
	V0 = V1 + 10 - 6
	return 0
}