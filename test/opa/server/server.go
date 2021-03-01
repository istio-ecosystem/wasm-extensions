package server

import (
	"fmt"
	"go/build"
	"os"
	"os/exec"
	"runtime"

	framework "istio.io/proxy/test/envoye2e/driver"
)

// OpaServer models an OPA server process.
type OpaServer struct {
	opaProcess   *os.Process
	RuleFilePath string
}

var _ framework.Step = &OpaServer{}

// Run starts an OPA server
func (o *OpaServer) Run(p *framework.Params) error {
	opaPath, err := downloadOpaServer()
	if err != nil {
		return err
	}

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
	o.opaProcess.Kill()
}

func downloadOpaServer() (string, error) {
	outputPath := fmt.Sprintf("%s/out/%s_%s", build.Default.GOPATH, runtime.GOOS, runtime.GOARCH)
	dst := fmt.Sprintf("%v/opa", outputPath)
	if _, err := os.Stat(dst); err == nil {
		return dst, nil
	}
	opaURL := "https://openpolicyagent.org/downloads/latest/opa_linux_amd64"
	fmt.Printf("download opa server to %v from %v", dst, opaURL)
	donwloadCmd := exec.Command("bash", "-c", fmt.Sprintf("curl -L -o %v %v", dst, opaURL))
	donwloadCmd.Stderr = os.Stderr
	donwloadCmd.Stdout = os.Stdout
	err := donwloadCmd.Run()
	if err != nil {
		return "", fmt.Errorf("fail to run opa download command: %v", err)
	}
	chmodCmd := exec.Command("bash", "-c", fmt.Sprintf("chmod 755 %v", dst))
	chmodCmd.Stderr = os.Stderr
	chmodCmd.Stdout = os.Stdout
	err = chmodCmd.Run()
	if err != nil {
		return "", fmt.Errorf("fail to chmod for opa: %v", err)
	}
	return dst, nil
}
