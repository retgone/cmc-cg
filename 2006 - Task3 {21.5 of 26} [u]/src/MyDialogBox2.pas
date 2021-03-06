unit MyDialogBox2;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm3 = class(TForm)
    Label1: TLabel;
    Edit1: TEdit;
    procedure Edit1KeyPress(Sender: TObject; var Key: Char);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form3: TForm3;

function GetSquare: Integer;

implementation

{$R *.dfm}

procedure TForm3.Edit1KeyPress(Sender: TObject; var Key: Char);
begin
  if Key = #13 then
    Form3.ModalResult := mrOk;
  if Key = #27 then
    Form3.ModalResult := mrCancel;
end;

function GetSquare: Integer;
begin
  Form3.Edit1.Text := '';
  if Form3.ShowModal = mrOk then
    Result := StrToInt(Form3.Edit1.Text)
  else
    Result := 0;
end;

end.
