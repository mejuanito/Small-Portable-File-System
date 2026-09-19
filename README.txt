To compile code use:

gcc main.c fs.c -o FS

And to run use: 

./FS

Commands are used in this format:

Createfs '#ofblocks'

Formatfs '#offilenames' '#ofDABTEntries'

Savefs 'filename'

Openfs 'filename'

List 

Put 'filename'

Get 'filename'

Remove 'filename'

Rename 'oldname' 'newname'

User 'file' 'name'

Link 'oldname' 'newname'

Unlink 'filename'