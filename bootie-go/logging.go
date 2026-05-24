package main

import (
	"github.com/charmbracelet/log"
	"github.com/charmbracelet/lipgloss"
)

const SuccessLevel = log.InfoLevel + 2

func init() {
	styles := log.DefaultStyles()
	styles.Levels[SuccessLevel] = lipgloss.NewStyle().
		SetString("SUCCESS").
		Bold(true).
		MaxWidth(7).
		Foreground(lipgloss.Color("86"))
	log.SetStyles(styles)
}
