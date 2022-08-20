package main

import (
	"io"
	"net/http"
	"runtime"
        "fmt"
	"C"
	"os"
)

func hello(w http.ResponseWriter, r *http.Request) {
	hostname, err := os.Hostname()
        if( err == nil ) {
		io.WriteString(w, "Hello world from " + runtime.Version() + " at " + hostname)
        }
}

func main() {
}

//export GoMain
func GoMain() {
	fmt.Println("Go version:", runtime.Version());
	http.HandleFunc("/", hello)
	http.ListenAndServe(":8000", nil)
}
