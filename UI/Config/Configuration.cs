﻿using Avalonia;
using Avalonia.Controls;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public partial class Configuration : ReactiveObject
	{
		private string _fileData = "";

		public string Version { get; set; } = "0.4.0";
		
		[Reactive] public VideoConfig Video { get; set; } = new();
		[Reactive] public AudioConfig Audio { get; set; } = new();
		[Reactive] public InputConfig Input { get; set; } = new();
		[Reactive] public EmulationConfig Emulation { get; set; } = new();
		[Reactive] public SnesConfig Snes { get; set; } = new();
		[Reactive] public NesConfig Nes { get; set; } = new();
		[Reactive] public GameboyConfig Gameboy { get; set; } = new();
		[Reactive] public PcEngineConfig PcEngine { get; set; } = new();
		[Reactive] public PreferencesConfig Preferences { get; set; } = new();
		[Reactive] public AudioPlayerConfig AudioPlayer { get; set; } = new();
		[Reactive] public DebugConfig Debug { get; set; } = new();
		[Reactive] public RecentItems RecentFiles { get; set; } = new();
		[Reactive] public VideoRecordConfig VideoRecord { get; set; } = new();
		[Reactive] public MovieRecordConfig MovieRecord { get; set; } = new();
		[Reactive] public HdPackBuilderConfig HdPackBuilder { get; set; } = new();
		[Reactive] public CheatWindowConfig Cheats { get; set; } = new();
		[Reactive] public NetplayConfig Netplay { get; set; } = new();
		[Reactive] public HistoryViewerConfig HistoryViewer { get; set; } = new();
		[Reactive] public MainWindowConfig MainWindow { get; set; } = new();
		
		public bool FirstRun { get; set; } = true;
		public DefaultKeyMappingType DefaultKeyMappings { get; set; } = DefaultKeyMappingType.Xbox | DefaultKeyMappingType.ArrowKeys;

		public Configuration()
		{
		}

		~Configuration()
		{
			//Try to save before destruction if we were unable to save at a previous point in time
			Save();
		}

		public void Save()
		{
			if(ConfigManager.DisableSaveSettings) {
				//Don't save to disk if command line option to disable setting updates was set
				return;
			}

			Serialize(ConfigManager.ConfigFile);
		}

		public void ApplyConfig()
		{
			Video.ApplyConfig();
			Audio.ApplyConfig();
			Input.ApplyConfig();
			Emulation.ApplyConfig();
			Gameboy.ApplyConfig();
			PcEngine.ApplyConfig();
			Nes.ApplyConfig();
			Snes.ApplyConfig();
			Preferences.ApplyConfig();
			AudioPlayer.ApplyConfig();
			Debug.ApplyConfig();
		}

		public void InitializeDefaults()
		{
			if(FirstRun) {
				Snes.InitializeDefaults(DefaultKeyMappings);
				Nes.InitializeDefaults(DefaultKeyMappings);
				Gameboy.InitializeDefaults(DefaultKeyMappings);
				PcEngine.InitializeDefaults(DefaultKeyMappings);
				FirstRun = false;
			}
			Preferences.InitializeDefaultShortcuts();
		}

		public static Configuration Deserialize(string configFile)
		{
			Configuration config;

			try {
				string fileData = File.ReadAllText(configFile);
				config = JsonSerializer.Deserialize<Configuration>(fileData, JsonHelper.Options) ?? new Configuration();
				config._fileData = fileData;
			} catch {
				config = new Configuration();
			}

			return config;
		}

		public void Serialize(string configFile)
		{
			try {
				string cfgData = JsonSerializer.Serialize(this, typeof(Configuration), JsonHelper.Options);
				if(_fileData != cfgData && !Design.IsDesignMode) {
					FileHelper.WriteAllText(configFile, cfgData);
					_fileData = cfgData;
				}
			} catch {
				//This can sometime fail due to the file being used by another Mesen instance, etc.
			}
		}

		public void RemoveObsoleteConfig()
		{
			//Clean up configuration to remove any obsolete values that existed in older versions
			for(int i = Preferences.ShortcutKeys.Count - 1; i >= 0; i--) {
				if(Preferences.ShortcutKeys[i].Shortcut >= EmulatorShortcut.LastValidValue) {
					Preferences.ShortcutKeys.RemoveAt(i);
				}
			}
		}
	}

	[Flags]
	public enum DefaultKeyMappingType
	{
		None = 0,
		Xbox = 1,
		Ps4 = 2,
		WasdKeys = 4,
		ArrowKeys = 8
	}
}