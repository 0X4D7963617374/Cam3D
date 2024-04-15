object Form1: TForm1
  Left = 0
  Top = 309
  Caption = 'Form1'
  ClientHeight = 904
  ClientWidth = 1128
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Position = poDesktopCenter
  OnCreate = FormCreate
  TextHeight = 13
  object Image1: TImage
    Left = 8
    Top = 39
    Width = 1024
    Height = 848
    Stretch = True
  end
  object Label1: TLabel
    Left = 672
    Top = 16
    Width = 93
    Height = 13
    Alignment = taCenter
    Caption = 'Nombre de pixel : 0'
  end
  object Button1: TButton
    Left = 8
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Connect'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 96
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Deconnect'
    TabOrder = 1
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 177
    Top = 12
    Width = 121
    Height = 21
    TabOrder = 2
    Text = '3100'
  end
  object Edit2: TEdit
    Left = 304
    Top = 12
    Width = 121
    Height = 21
    TabOrder = 3
    Text = '10000'
  end
  object Button3: TButton
    Left = 534
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Option'
    TabOrder = 4
    OnClick = Button3Click
  end
  object CheckBox1: TCheckBox
    Left = 431
    Top = 16
    Width = 97
    Height = 17
    Caption = 'RGB'
    Checked = True
    State = cbChecked
    TabOrder = 5
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 1
    OnTimer = Timer1Timer
    Left = 184
    Top = 8
  end
end
