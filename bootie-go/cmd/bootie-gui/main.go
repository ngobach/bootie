package main

import (
	. "modernc.org/tk9.0"
)

func main() {
	// Set the main window title
	App.WmTitle("Bootie CGO-Free GUI")

	// Create a label to display text
	lbl := TLabel(Txt("Welcome to Bootie! Click the button below."))

	// Create a themed button ("Say Hello")
	btn := TButton(
		Txt("Say Hello"),
		Command(func() {
			// Change the label text when clicked
			lbl.Configure(Txt("Hello, World! This is running 100% CGO-free!"))
		}),
	)

	// Layout the widgets using the Pack manager
	Pack(lbl, Padx("20m"), Pady("10m"))
	Pack(btn, Pady("10m"))

	// Run the main event loop
	App.Wait()
}
