package main

import (
        "runtime"
        "fmt"
	"C"
)

func main() {
}

//export GoMain
func GoMain() {
	fmt.Println("Hello, 世界");
	fmt.Println("Go version:", runtime.Version());
}
