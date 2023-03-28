package main

import (
	"bufio"
	"fmt"
	"log"
	"os"

	"github.com/ua-parser/uap-go/uaparser"
)

func main() {
	// uagent := "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-us; Silk/1.1.0-80) AppleWebKit/533.16 (KHTML, like Gecko) Version/5.0 Safari/533.16 Silk-Accelerated=true"
	parser, err := uaparser.New("./uap-core/regexes.yaml")
	if err != nil {
		log.Fatal(err)
	}

	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		client := parser.Parse(line)

		fmt.Print(client.UserAgent.Family)
		fmt.Print(client.UserAgent.Major)
		fmt.Print(client.UserAgent.Minor)
		fmt.Print(client.UserAgent.Patch)
		fmt.Print(client.Os.Family)
		fmt.Print(client.Os.Major)
		fmt.Print(client.Os.Minor)
		fmt.Print(client.Os.Patch)
		fmt.Print(client.Os.PatchMinor)
		fmt.Print(client.Device.Family)
		fmt.Println()
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}
}

