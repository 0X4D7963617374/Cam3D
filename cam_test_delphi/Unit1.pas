unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ExtCtrls, Math, Generics.Collections;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Image1: TImage;
    Timer1: TTimer;
    Edit1: TEdit;
    Edit2: TEdit;
    Button3: TButton;
    CheckBox1: TCheckBox;
    Label1: TLabel;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure Button3Click(Sender: TObject);
  private
    { Déclarations privées }
  public
    { Déclarations publiques }
  end;

const
  BUFFER_SIZE = 1;

type
  TImageBuffer = record
    Images: array [0 .. BUFFER_SIZE - 1] of TArray<Integer>;
    Current: Integer;
  end;

var
  Form1: TForm1;
  hDLL: THandle;

  Connect: procedure; stdcall;
  Disconnect: procedure; stdcall;
  SetOptions: procedure(Arc: Integer; dist: Integer; RGB: boolean); stdcall;
  ReceiveArray: procedure(arr: PInteger; len: Integer); stdcall;
  depthArrayDyna: TArray<Integer>;
  bmp: TBitmap;

  lastUpdatedRect: TRect;

  Buffer: TImageBuffer;

  depthArray: TArray<Integer>;

implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);
var
  toto: Integer;
  titi: Integer;
begin
  toto := strtoint(Edit1.Text);
  titi := strtoint(Edit2.Text);
  hDLL := LoadLibrary('My_Cam3D.dll');
  if hDLL <> 0 then
  begin
    @Connect := GetProcAddress(hDLL, 'Connect');
    if Assigned(Connect) then
    begin
      Connect();
    end
    else
      ShowMessage('Impossible de trouver la fonction Connect');

    @ReceiveArray := GetProcAddress(hDLL, 'ReceiveArray');
    if not Assigned(ReceiveArray) then
      ShowMessage('Impossible de trouver la fonction ReceiveArray');

    Timer1.Enabled := True;
  end
  else
    ShowMessage('Impossible de charger la DLL');
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  Timer1.Enabled := False;
  if hDLL <> 0 then
  begin
    @Disconnect := GetProcAddress(hDLL, 'Disconnect');
    if Assigned(Disconnect) then
    begin
      Disconnect();
    end
    else
      ShowMessage('Impossible de trouver la fonction Disconnect');

    FreeLibrary(hDLL);
    hDLL := 0;
  end
  else
    ShowMessage('La DLL n''est pas chargée');
end;

procedure TForm1.Button3Click(Sender: TObject);
var
  toto: Integer;
  titi: Integer;
begin
  toto := strtoint(Edit1.Text);
  titi := strtoint(Edit2.Text);
  hDLL := LoadLibrary('My_Cam3D.dll');
  if hDLL <> 0 then
  begin
    @SetOptions := GetProcAddress(hDLL, 'SetOptions');
    if Assigned(SetOptions) then
    begin
      SetOptions(toto, titi, Form1.CheckBox1.Checked);
    end
    else
      ShowMessage('Impossible de trouver la fonction Connect');

  end
  else
    ShowMessage('Impossible de charger la DLL');
end;

function GetClosestNonBlackPixel(x, y, Width, Height: Integer; UsedPixels: TList<Integer>): Integer;
var
  dx, dy, distance, i, j: Integer;
begin
  Result := 0;
  distance := MaxInt;

  for i := Max(0, y - 5) to Min(Height - 1, y + 5) do
  begin
    for j := Max(0, x - 5) to Min(Width - 1, x + 5) do
    begin
      if (depthArray[i * Width + j] <> 0) and (UsedPixels.IndexOf(i * Width + j) = -1) and ((abs(y - i) + abs(x - j)) < distance) then
      begin
        distance := abs(y - i) + abs(x - j);
        Result := depthArray[i * Width + j];
      end;
    end;
  end;

  if Result <> 0 then
    UsedPixels.Add(y * Width + x);
end;

procedure BilinearInterpolation(var Pixels: TArray<Integer>; Width, Height: Integer);
var
  i, j, x, y, count, sum: Integer;
  neighborhood: array [0 .. 3] of Integer;
begin
  for i := 0 to Height - 1 do
  begin
    for j := 0 to Width - 1 do
    begin
      if Pixels[i * Width + j] = 0 then
      begin
        count := 0;
        sum := 0;
        for y := Max(0, i - 1) to Min(i + 1, Height - 1) do
        begin
          for x := Max(0, j - 1) to Min(j + 1, Width - 1) do
          begin
            if not((x = j) and (y = i)) and (Pixels[y * Width + x] <> 0) then
            begin
              sum := sum + Pixels[y * Width + x];
              count := count + 1;
            end;
          end;
        end;
        if count > 0 then
          Pixels[i * Width + j] := sum div count;
      end;
    end;
  end;
end;

procedure InitialiseBuffer(Width, Height: Integer);
var
  i: Integer;
begin
  for i := 0 to BUFFER_SIZE - 1 do
    SetLength(Buffer.Images[i], Width * Height);
  Buffer.Current := 0;
end;

procedure UpdateBuffer(const Image: TArray<Integer>);
begin
  Buffer.Images[Buffer.Current] := Copy(Image);
  Buffer.Current := (Buffer.Current + 1) mod BUFFER_SIZE;
end;

function SearchBuffer(x, y, Width: Integer): Integer;
var
  i, j: Integer;
begin
  Result := 0;
  for i := 0 to BUFFER_SIZE - 1 do
  begin
    j := (BUFFER_SIZE + Buffer.Current - i) mod BUFFER_SIZE;
    if Buffer.Images[j][y * Width + x] <> 0 then
    begin
      Result := Buffer.Images[j][y * Width + x];
      Break;
    end;
  end;
end;

procedure BytesArrayToBitmapDepth(const BytesArray: array of Integer; const Width, Height: Integer; Bitmap: TBitmap);
var
  Line: PByteArray;
  i, j, k: Integer;
  RGBValue: Integer;
begin
  Bitmap.SetSize(Width, Height);
  Bitmap.PixelFormat := pf24bit;
  k := 0;
  for i := 0 to Height - 1 do
  begin
    Line := Bitmap.ScanLine[i];
    for j := 0 to Width - 1 do
    begin
      RGBValue := BytesArray[i * Width + j];

      Line[j * 3] := (RGBValue and $0000FF);
      Line[j * 3 + 1] := (RGBValue and $00FF00) shr 8;
      Line[j * 3 + 2] := (RGBValue and $FF0000) shr 16;
      if RGBValue <> 0 then
        inc(k);
    end;
  end;
  Form1.Label1.Caption := 'Nombre de pixel : ' + inttostr(k);
end;

procedure InterpolateBlackPixels(var BytesArray: TArray<Integer>; const Width, Height: Integer);
var
  i, j: Integer;
begin
  for i := 0 to Height - 1 do
  begin
    for j := 0 to Width - 1 do
    begin
      if BytesArray[i * Width + j] = 0 then
      begin
        if (i > 0) and (BytesArray[(i - 1) * Width + j] <> 0) then
        begin
          BytesArray[i * Width + j] := BytesArray[(i - 1) * Width + j];
        end
        else if (i < Height - 1) and (BytesArray[(i + 1) * Width + j] <> 0) then
        begin
          BytesArray[i * Width + j] := BytesArray[(i + 1) * Width + j];
        end
        else if (j > 0) and (BytesArray[i * Width + j - 1] <> 0) then
        begin
          BytesArray[i * Width + j] := BytesArray[i * Width + j - 1];
        end
        else if (j < Width - 1) and (BytesArray[i * Width + j + 1] <> 0) then
        begin
          BytesArray[i * Width + j] := BytesArray[i * Width + j + 1];
        end;
      end;
    end;
  end;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
var
  i, j: Integer;
  localDepthArray: TArray<Integer>;
  UsedPixels: TList<Integer>;
begin
  if Assigned(ReceiveArray) then
  begin
    SetLength(localDepthArray, Length(depthArray));
    ReceiveArray(@localDepthArray[0], Length(localDepthArray));
    UpdateBuffer(localDepthArray);

    UsedPixels := TList<Integer>.Create;
    try
      for i := 0 to bmp.Height - 1 do
      begin
        for j := 0 to bmp.Width - 1 do
        begin
        end;
      end;
      BytesArrayToBitmapDepth(localDepthArray, bmp.Width, bmp.Height, bmp);
      Image1.Picture.Bitmap := bmp;
    finally
      UsedPixels.Free;
    end;
  end;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  bmp := TBitmap.Create;
  bmp.PixelFormat := pf24bit;
  bmp.Width := 512;
  bmp.Height := 424;
  SetLength(depthArray, bmp.Width * bmp.Height);
  InitialiseBuffer(bmp.Width, bmp.Height);
end;

end.
