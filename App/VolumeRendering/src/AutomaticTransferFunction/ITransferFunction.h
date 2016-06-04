#ifndef I_TRANSFER_FUNCTION_H
#define I_TRANSFER_FUNCTION_H

class ITransferFunction
{
public:
  /// <summary>
  /// The implementation of this function must generate a
  /// transfer function at the given path.
  /// A transfer function file has the extension "tf1d" and
  /// its structure is as follows:
  /// - First line: Always considered a comment.
  /// - Second line: Number 'nc' of color attributes.
  /// - Next 'nc' lines: R G B V. Where R, G and B are the
  /// rgb float components associated to a value V.
  /// - Next line: Number 'no' of opacity attribute.
  /// - Next 'no' lines: O V. Where O is an opacity
  /// associated to a value V.
  /// </summary>
  /// <param name="path">The transfer function file's path.</param>
  virtual bool Generate(const char* path) = 0;
};

#endif