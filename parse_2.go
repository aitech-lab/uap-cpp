package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"time"

	"github.com/ua-parser/uap-go/uaparser"
)

func main() {
	// uagent := "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-us; Silk/1.1.0-80) AppleWebKit/533.16 (KHTML, like Gecko) Version/5.0 Safari/533.16 Silk-Accelerated=true"
	parser, err := uaparser.New("./uap-core/regexes.yaml")
	if err != nil {
		log.Fatal(err)
	}
	in := make(chan string)
	out := make(chan string)

	go func() {
		scanner := bufio.NewScanner(os.Stdin)
		if err := scanner.Err(); err != nil {
			fmt.Fprintln(os.Stderr, "reading standard input:", err)
		}
		for scanner.Scan() {
			in <- scanner.Text()
		}
		close(in)

	}()

	for i := 0; i <= 100; i++ {
		go func() {
			for line := range in {
				client := parser.Parse(line)
				out <- client.UserAgent.Family
				// fmt.Print(client.UserAgent.Major)
				// fmt.Print(client.UserAgent.Minor)
				// fmt.Print(client.UserAgent.Patch)
				// fmt.Print(client.Os.Family)
				// fmt.Print(client.Os.Major)
				// fmt.Print(client.Os.Minor)
				// fmt.Print(client.Os.Patch)
				// fmt.Print(client.Os.PatchMinor)
				// fmt.Print(client.Device.Family)
			}
		}()
	}

loop:
	for {
		select {
		case o := <-out:
			fmt.Println(o)
		case <-time.After(1000 * time.Millisecond):
			close(out)
			break loop
		}
	}
}
