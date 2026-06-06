package main

import (
	"fmt"
	"go.bytecodealliance.org/cm"
	"unsafe"
	httphandler "wassel-go-plugin/internal/wassel/foundation/http-handler"
	httptypes "wassel-go-plugin/internal/wasi/http/types"
)

func init() {
	httphandler.Exports.HandleRequest = func(
		request httphandler.IncomingRequest,
		responseOut httphandler.ResponseOutparam,
	) {
		path := "/"
		pathOpt := request.PathWithQuery()
		if p := pathOpt.Some(); p != nil {
			path = *p
		}

		content := []byte("Hello, " + path + "!")
		contentLength := []byte(fmt.Sprintf("%d", len(content)))

		toFieldValue := func(b []byte) httptypes.FieldValue {
	     	       l := cm.ToList[[]uint8, uint8](b)
		       return *(*httptypes.FieldValue)(unsafe.Pointer(&l))
		}

		fields := httptypes.NewFields()
		fields.Append("content-length", toFieldValue(contentLength))

		out := httptypes.NewOutgoingResponse(fields)
		bodyResult := out.Body()
		body := bodyResult.OK()

		streamResult := body.Write()
		stream := streamResult.OK()
		stream.BlockingWriteAndFlush(cm.ToList[[]uint8, uint8](content))
		stream.ResourceDrop()

		httptypes.OutgoingBodyFinish(*body, cm.None[httptypes.Trailers]())
		httptypes.ResponseOutparamSet(
			responseOut,
			cm.OK[cm.Result[httptypes.ErrorCodeShape, httptypes.OutgoingResponse, httptypes.ErrorCode]](out),
		)
	}
}

func main() {}