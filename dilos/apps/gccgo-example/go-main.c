/* 
 * When compiling a Go program with gccgo, the main() is normally picked up
 * from libgobegin.a. In the past this library was compiled without -fPIC
 * so we couldn't use it in OSv, but in more recent gccgo this was fixed
 * (see https://patchwork.ozlabs.org/patch/464423/) and now we can use it.
 *
 * Still, we need to make sure the linker picks up main() from that library.
 * When compiling an executable, it knows it should. But when compiling a
 * a shared object, it doesn't know. We need to refer to main() to force it
 * being included.
 */
extern int main();
void *force_main_to_be_included = main;
