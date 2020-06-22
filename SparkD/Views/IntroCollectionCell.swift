//
//  IntroCollectionCell.swift
//  SparkD
//
//  Created by com on 6/13/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class IntroCollectionCell: UICollectionViewCell {
    @IBOutlet weak var imgBack: UIImageView!
    @IBOutlet weak var lblNote: UILabel!
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        imgBack.frame = CGRect(x: 0, y: 0, width: frame.width, height: frame.width*666/840)
        lblNote.frame = CGRect(x: 0, y: imgBack.frame.maxY, width: frame.width, height: 80)
    }
}
