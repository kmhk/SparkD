//
//  TestProcedureVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class TestProcedureVC: UIViewController {

    @IBOutlet weak var collectionView: UICollectionView!
    @IBOutlet weak var btnStart: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "Test Procedure")
        navigationItem.titleView = titleView
        
        btnStart.roundSolidButton(title: "Start")
        
        let backButton = UIBarButtonItem(barButtonSystemItem: .stop, target: self, action: #selector(btnBack(_:)))
        navigationItem.rightBarButtonItem = backButton
    }
    

    // MARK: Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "segueDetail" {
            let vc = segue.destination as! TestProcedurePageVC
            vc.curPage = (sender as! IndexPath).row
        }
    }
    
    @objc func btnBack(_ sender: Any) {
        navigationController?.dismiss(animated: true, completion: nil)
    }

    @IBAction func btnStartTapped(_ sender: Any) {
        if let vc = AddTimerController.addTimer() {
            present(vc, animated: true, completion: nil)
        } else {
            let vc = UIAlertController(title: nil, message: "You can add new timer in 2 mins again", preferredStyle: .alert)
            vc.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
            present(vc, animated: true, completion: nil)
        }
    }
}

// MARK: -
extension TestProcedureVC: UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 9
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "TestProcedureCell", for: indexPath) as! TestProcedureCollectionViewCell
        
        cell.containerView.frame = CGRect(x: 2, y: 2, width: cell.frame.width - 4, height: cell.frame.width - 4)
        cell.containerView.roundGradientView()
        
        let imageName = String(format: "tutorial_step%d", indexPath.row + 1)
        let image = UIImage(named: imageName)
        
        cell.imageView.image = image
        cell.imageView.frame = CGRect(x: 0, y: 0, width: cell.containerView.frame.width / 3 * 2, height: cell.containerView.frame.width / 3 * 2)
        cell.imageView.center = cell.containerView.center
        cell.title.text = String(format: "Step %d", indexPath.row + 1)
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        let width = view.frame.width / 3 - 20
        return CGSize(width: width, height: width + 30)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, insetForSectionAt section: Int) -> UIEdgeInsets {
        return UIEdgeInsets(top: 5, left: 5, bottom: 5, right: 5)
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        performSegue(withIdentifier: "segueDetail", sender: indexPath)
    }
}
