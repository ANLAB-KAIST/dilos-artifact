#include <magick/studio.h>
#include <wand/MagickWand.h>

int main(int argc,char **argv) {
	ExceptionInfo *exception;
	ImageInfo *image_info;
	MagickBooleanType status;

	MagickCoreGenesis(*argv, MagickTrue);
	exception = AcquireExceptionInfo();
	image_info = AcquireImageInfo();
	status = MagickCommandGenesis(image_info,
		ConvertImageCommand,
		argc, argv,
		(char **) NULL, exception);
	image_info = DestroyImageInfo(image_info);
	exception = DestroyExceptionInfo(exception);
	MagickCoreTerminus();
	return(status != MagickFalse ? 0 : 1);
}
