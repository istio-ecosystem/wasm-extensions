package server

import (
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"

	framework "istio.io/proxy/test/envoye2e/driver"
)

// OpaServer models an OPA server process.
type OpaServer struct {
	opaProcess   *os.Process
	tmpDir       string
	RuleFilePath string
}

var _ framework.Step = &OpaServer{}

// Run starts an OPA server
func (o *OpaServer) Run(p *framework.Params) error {
	opaPath, err := downloadOpaServer()
	if err != nil {
		return err
	}
	o.tmpDir = filepath.Dir(opaPath)

	// Run Opa Server with given rule file
	opaServerCmd := fmt.Sprintf("%v run --server --log-level debug %v", opaPath, o.RuleFilePath)
	fmt.Printf("start opa server: %v\n", opaServerCmd)
	opaCmd := exec.Command("bash", "-c", opaServerCmd)
	opaCmd.Stderr = os.Stderr
	opaCmd.Stdout = os.Stdout
	err = opaCmd.Start()
	if err != nil {
		return err
	}
	o.opaProcess = opaCmd.Process
	return nil
}

// Cleanup closes an OPA server process.
func (o *OpaServer) Cleanup() {
	os.Remove(o.tmpDir)
	o.opaProcess.Kill()
}

func downloadOpaServer() (string, error) {
	tmpdDir, err := ioutil.TempDir("", "opa-")
	dst := fmt.Sprintf("%s/%s", tmpdDir, "opa")

	opaURL := "https://openpolicyagent.org/downloads/latest/opa_linux_amd64"
	resp, err := http.Get(opaURL)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	if err != nil {
		return "", fmt.Errorf("fail to download opa: %v", err)
	}
	outFile, err := os.OpenFile(dst, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
	if err != nil {
		return "", err
	}
	defer outFile.Close()

	// Write the body to file
	_, err = io.Copy(outFile, resp.Body)

	return dst, nil
}
